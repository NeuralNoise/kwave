/***************************************************************************
       PluginManager.cpp -  manager class for Kwave's plugins
			     -------------------
    begin                : Sun Aug 27 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include <QMutableListIterator>

#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "libkwave/KwaveMultiPlaybackSink.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/PluginContext.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

#include "libgui/MessageBox.h"

#include "KwaveApp.h"
#include "KwaveSplash.h"
#include "TopWidget.h"
#include "SignalManager.h"
#include "UndoAction.h"
#include "UndoModifyAction.h"
#include "UndoTransactionGuard.h"

#include "PluginManager.h"

//***************************************************************************
//***************************************************************************
PluginManager::PluginDeleter::PluginDeleter(KwavePlugin *plugin, void *handle)
  :QObject(), m_plugin(plugin), m_handle(handle)
{
}

//***************************************************************************
PluginManager::PluginDeleter::~PluginDeleter()
{
    // delete the plugin, this should also remove everything it has allocated
    if (m_plugin) delete m_plugin;

    // now the handle of the shared object can be cleared too
    dlclose(m_handle);
}

//***************************************************************************
//***************************************************************************

// static initializers

QMap<QString, QString> PluginManager::m_plugin_files;

PluginManager::PluginList PluginManager::m_unique_plugins;

static QList<PlaybackDeviceFactory *> m_playback_factories;

//***************************************************************************
PluginManager::PluginManager(TopWidget &parent)
    :m_loaded_plugins(), m_running_plugins(),
     m_top_widget(parent)
{
    // use all unique plugins
    // this does nothing on the first instance, all other instances
    // will probably find a non-empty list
    foreach (KwavePluginPointer p, m_unique_plugins) {
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) {
	    p->use();

	    // maybe we will become responsible for releasing
	    // the plugin (when it is in use but the plugin manager
	    // who has created it is already finished)
	    connect(p,    SIGNAL(sigClosed(KwavePlugin *)),
	            this, SLOT(pluginClosed(KwavePlugin *)));
	}
    }
}

//***************************************************************************
PluginManager::~PluginManager()
{
    // inform all plugins and client windows that we close now
    emit sigClosed();

    // wait until all plugins are really closed
    this->sync();

    // release all unique plugins
    while (!m_unique_plugins.isEmpty()) {
	KwavePluginPointer p = m_unique_plugins.takeLast();
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) p->release();
    }

    // release all own persistent plugins
    while (!m_loaded_plugins.isEmpty()) {
	KwavePluginPointer p = m_loaded_plugins.takeLast();
	Q_ASSERT(p);
	if (p && p->isPersistent()) p->release();
    }

    // release all own plugins that are left
    while (!m_loaded_plugins.isEmpty()) {
	KwavePluginPointer p = m_loaded_plugins.takeLast();
	Q_ASSERT(p);
	if (p) p->release();
    }
}

//***************************************************************************
void PluginManager::loadAllPlugins()
{
    // Try to load all plugins. This has to be called only once per
    // instance of the main window!
    // NOTE: this also implicitely makes it resident if it is persistent or unique
    foreach (QString name, m_plugin_files.keys()) {
	KwavePluginPointer plugin = loadPlugin(name);
	if (plugin) {
// 	    QString state = "";
// 	    if (plugin->isPersistent()) state += "(persistent)";
// 	    if (plugin->isUnique()) state += "(unique)";
// 	    if (!state.length()) state = "(normal)";
// 	    qDebug("PluginManager::loadAllPlugins(): plugin '"+
// 		   plugin->name()+"' "+state);
	    if (!plugin->isUnique() && !plugin->isPersistent()) {
		// remove it again if it is neither unique nor persistent
		plugin->release();
	    }
	} else {
	    // loading failed => remove it from the list
	    qWarning("PluginManager::loadAllPlugins(): removing '%s' "\
	            "from list", name.toLocal8Bit().data());
	    m_plugin_files.remove(name);
	}
    }
}

//***************************************************************************
KwavePlugin *PluginManager::loadPlugin(const QString &name)
{

    // first find out if the plugin is already loaded and persistent
    foreach (KwavePluginPointer p, m_loaded_plugins) {
	if (p && p->isPersistent() && (p->name() == name)) {
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('%s')"\
	           "-> returning pointer to persistent",
	           name.toLocal8Bit().data());
	    return p;
	}
    }

    // find out if the plugin is already loaded and unique
    foreach (KwavePluginPointer p, m_unique_plugins) {
	if (p && (p->name() == name)) {
	    Q_ASSERT(p->isUnique());
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('%s')"\
	           "-> returning pointer to unique+persistent",
	           name.toLocal8Bit().data());
	    return p;
	}
    }

    // show a warning and abort if the plugin is unknown
    if (!(m_plugin_files.contains(name))) {
	QString message =
	    i18n("oops, plugin '%1' is unknown or invalid!", name);
	Kwave::MessageBox::error(&m_top_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }
    QString &filename = m_plugin_files[name];

    // try to get the file handle of the plugin's binary
    void *handle = dlopen(filename.toLocal8Bit(), RTLD_NOW);
    if (!handle) {
	QString message = i18n("unable to load the file \n'%1'\n"\
	                       " that contains the plugin '%2' !",
	                       filename, name);
	Kwave::MessageBox::error(&m_top_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }

    // get the loader function
    KwavePlugin *(*plugin_loader)(const PluginContext *c) = 0;

    // hardcoded, should always work when the
    // symbols are declared as extern "C"
    const char *sym_version = "version";
    const char *sym_author  = "author";
    const char *sym_loader  = "load";

    // get the plugin's author
    const char *author = "";
    const char **pauthor = (const char **)dlsym(handle, sym_author);
    Q_ASSERT(pauthor);
    if (pauthor) author=*pauthor;
    if (!author) author = i18n("(unknown)").toLocal8Bit();

    // get the plugin's version string
    const char *version = "";
    const char **pver = (const char **)dlsym(handle, sym_version);
    Q_ASSERT(pver);
    if (pver) version=*pver;
    if (!version) version = i18n("(unknown)").toLocal8Bit();

    plugin_loader =
        (KwavePlugin *(*)(const PluginContext *))dlsym(handle, sym_loader);
    Q_ASSERT(plugin_loader);
    if (!plugin_loader) {
	// plugin is null, out of memory or not found
	qWarning("PluginManager::loadPlugin('%s'): "\
		"plugin does not contain a loader, "\
		"maybe it is damaged or the wrong version?",
		name.toLocal8Bit().data());
	dlclose(handle);
	return 0;
    }

    PluginContext context(
	m_top_widget.getKwaveApp(),
	*this,
	0, // MenuManager   *menu_mgr,
	m_top_widget,
	handle,
	name,
	version,
	author
    );

    // call the loader function to create an instance
    KwavePlugin *plugin = (*plugin_loader)(&context);
    Q_ASSERT(plugin);
    if (!plugin) {
	qWarning("PluginManager::loadPlugin('%s'): out of memory",
	         name.toLocal8Bit().data());
	dlclose(handle);
	return 0;
    }

    if (plugin->isUnique()) {
	// append unique plugins to the global list of unique plugins
	m_unique_plugins.append(plugin);
	plugin->use();
    } else {
	// append the plugin into our list of loaded plugins
	m_loaded_plugins.append(plugin);
	if (plugin->isPersistent()) {
	    // increment the usage if it is persistent, we will
	    // only load it once in this instance
	    plugin->use();
	}
    }

    // connect all necessary signals/slots
    connectPlugin(plugin);

    // get the last settings and call the "load" function
    // now the plugin is present and loaded
    QStringList last_params = loadPluginDefaults(name, plugin->version());
    plugin->load(last_params);

    return plugin;
}

//***************************************************************************
int PluginManager::executePlugin(const QString &name, QStringList *params)
{
    QString command;
    int result = 0;

    // synchronize: wait until any currently running plugins are done
    sync();

    // load the new plugin
    KwavePluginPointer plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    if (params) {
	// parameters were specified -> call directly
	// without setup dialog
	result = plugin->start(*params);

	// maybe the start() function has called close() ?
	if (!m_loaded_plugins.contains(plugin)) {
	    qDebug("PluginManager: plugin closed itself in start()"); // ###
	    result = -1;
	    plugin = 0;
	}

	if (plugin && (result >= 0)) {
	    plugin->use(); // plugin->release() will be called after run()
	    plugin->execute(*params);
	}
    } else {
	// load previous parameters from config
	QStringList last_params = loadPluginDefaults(name, plugin->version());

	// call the plugin's setup function
	params = plugin->setup(last_params);
	if (params) {
	    // we have a non-zero parameter list, so
	    // the setup function has not been aborted.
	    // Now we can create a command string and
	    // emit a new command.

	    // store parameters for the next time
	    savePluginDefaults(name, plugin->version(), *params);

	    // We DO NOT call the plugin's "execute"
	    // function directly, as it should be possible
	    // to record all function calls in the
	    // macro recorder
	    command = "plugin:execute(";
	    command += name;
	    for (int i = 0; i < params->count(); i++) {
		command += ", ";
		command += params->at(i);
	    }
	    delete params;
	    command += ")";
//	    qDebug("PluginManager: command='%s'",command.data());
	}
    }

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached
    if (plugin && !plugin->isPersistent()) plugin->release();

    // emit a command, let the toplevel window (and macro recorder) get
    // it and call us again later...
    if (command.length()) emit sigCommand(command);

    return result;
}

//***************************************************************************
bool PluginManager::onePluginRunning()
{
    foreach (KwavePluginPointer plugin, m_loaded_plugins)
	if (plugin && plugin->isRunning()) return true;
    return false;
}

//***************************************************************************
void PluginManager::sync()
{
    while (onePluginRunning()) {
	pthread_yield();
    }
}

//***************************************************************************
int PluginManager::setupPlugin(const QString &name)
{
    // load the plugin
    KwavePlugin* plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    // now the plugin is present and loaded
    QStringList last_params = loadPluginDefaults(name, plugin->version());

    // call the plugin's setup function
    QStringList *params = plugin->setup(last_params);
    if (params) {
	// we have a non-zero parameter list, so
	// the setup function has not been aborted.
	savePluginDefaults(name, plugin->version(), *params);
	delete params;
    } else return -1;

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached and is not persistent
    if (!plugin->isPersistent()) plugin->release();

    return 0;
}

//***************************************************************************
FileInfo &PluginManager::fileInfo()
{
    return m_top_widget.signalManager().fileInfo();
}

//***************************************************************************
QStringList PluginManager::loadPluginDefaults(const QString &name,
	const QString &version)
{
    QString def_version;
    QString section("plugin ");
    QStringList list;
    section += name;

    Q_ASSERT(KGlobal::config());
    if (!KGlobal::config()) return list;
    KConfigGroup cfg = KGlobal::config()->group(section);

    cfg.sync();

    def_version = cfg.readEntry("version");
    if (!def_version.length()) {
	return list;
    }
    if (!(def_version == version)) {
	qDebug("PluginManager::loadPluginDefaults: "\
	    "plugin '%s': defaults for version '%s' not loaded, found "\
	    "old ones of version '%s'.", name.toLocal8Bit().data(),
	    version.toLocal8Bit().data(), def_version.toLocal8Bit().data());
	return list;
    }

    list = cfg.readEntry("defaults").split(',');
    return list;
}

//***************************************************************************
void PluginManager::savePluginDefaults(const QString &name,
                                       const QString &version,
                                       QStringList &params)
{
    QString section("plugin ");
    section += name;

    Q_ASSERT(KGlobal::config());
    if (!KGlobal::config()) return;
    KConfigGroup cfg = KGlobal::config()->group(section);

    cfg.sync();
    cfg.writeEntry("version", version);
    cfg.writeEntry("defaults", params);
    cfg.sync();
}

//***************************************************************************
unsigned int PluginManager::signalLength()
{
    return m_top_widget.signalManager().length();
}

//***************************************************************************
double PluginManager::signalRate()
{
    return m_top_widget.signalManager().rate();
}

//***************************************************************************
const QList<unsigned int> PluginManager::selectedTracks()
{
    return m_top_widget.signalManager().selectedTracks();
}

//***************************************************************************
unsigned int PluginManager::selectionStart()
{
    return m_top_widget.signalManager().selection().first();
}

//***************************************************************************
unsigned int PluginManager::selectionEnd()
{
    return m_top_widget.signalManager().selection().last();
}

//***************************************************************************
void PluginManager::selectRange(unsigned int offset, unsigned int length)
{
    m_top_widget.signalManager().selectRange(offset, length);
}

//***************************************************************************
SampleWriter *PluginManager::openSampleWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    SignalManager &manager = m_top_widget.signalManager();
    return manager.openSampleWriter(track, mode, left, right, true);
}

//***************************************************************************
Kwave::SampleSink *PluginManager::openMultiTrackPlayback(
    unsigned int tracks, const QString *name)
{
    QString device_name;

    // locate the corresponding playback device factory (plugin)
    device_name = (name) ? QString(*name) : QString("");
    PlayBackDevice *device = 0;
    PlaybackDeviceFactory *factory = 0;
    foreach (PlaybackDeviceFactory *f, m_playback_factories) {
	Q_ASSERT(f);
	if (f && f->supportsDevice(device_name)) {
	    factory = f;
	    break;
	}
    }
    if (!factory) return 0;

    // open the playback device with it's default parameters
    device = factory->openDevice(device_name, tracks);
    if (!device) return 0;

    // create the multi track playback sink
    Kwave::SampleSink *sink = new Kwave::MultiPlaybackSink(tracks, device);
    Q_ASSERT(sink);
    return sink;
}

//***************************************************************************
PlaybackController &PluginManager::playbackController()
{
    return m_top_widget.signalManager().playbackController();
}

//***************************************************************************
void PluginManager::enqueueCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
void PluginManager::signalClosed()
{
    emit sigClosed();
}

//***************************************************************************
void PluginManager::pluginClosed(KwavePlugin *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!m_loaded_plugins.isEmpty() || !m_unique_plugins.isEmpty());
    if (!p) return;

    // disconnect the signals to avoid recursion
    disconnectPlugin(p);

    if (m_loaded_plugins.contains(p)) m_loaded_plugins.removeAll(p);
    if (m_unique_plugins.contains(p)) m_unique_plugins.removeAll(p);

    // schedule the deferred delete/unload of the plugin
    void *handle = p->handle();
    Q_ASSERT(handle);
    PluginDeleter *delete_later = new PluginDeleter(p, handle);
    Q_ASSERT(delete_later);
    if (delete_later) delete_later->deleteLater();

//    qDebug("PluginManager::pluginClosed(): done");
}

//***************************************************************************
void PluginManager::pluginStarted(KwavePlugin *p)
{
    Q_ASSERT(p);
    if (!p) return;

    // the plugin is running -> increase the usage count in order to
    // prevent our lists from containing invalid entries
    p->use();

    // add the plugin to the list of running plugins
    m_running_plugins.append(p);
}

//***************************************************************************
void PluginManager::pluginDone(KwavePlugin *p)
{
    Q_ASSERT(p);
    if (!p) return;

    // remove the plugin from the list of running plugins
    m_running_plugins.removeAll(p);

    // release the plugin, at least we do no longer need it
    p->release();
}

//***************************************************************************
void PluginManager::connectPlugin(KwavePlugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    if (!plugin->isUnique()) {
	connect(this, SIGNAL(sigClosed()),
	        plugin, SLOT(close()));
    }

    connect(plugin, SIGNAL(sigClosed(KwavePlugin *)),
	    this, SLOT(pluginClosed(KwavePlugin *)));

    connect(plugin, SIGNAL(sigRunning(KwavePlugin *)),
	    this, SLOT(pluginStarted(KwavePlugin *)));

    connect(plugin, SIGNAL(sigDone(KwavePlugin *)),
	    this, SLOT(pluginDone(KwavePlugin *)));
}

//***************************************************************************
void PluginManager::disconnectPlugin(KwavePlugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    disconnect(plugin, SIGNAL(sigDone(KwavePlugin *)),
	       this, SLOT(pluginDone(KwavePlugin *)));

    disconnect(plugin, SIGNAL(sigRunning(KwavePlugin *)),
	       this, SLOT(pluginStarted(KwavePlugin *)));

    disconnect(this, SIGNAL(sigClosed()),
	       plugin, SLOT(close()));

    if (!plugin->isUnique()) {
	disconnect(plugin, SIGNAL(sigClosed(KwavePlugin *)),
	           this, SLOT(pluginClosed(KwavePlugin *)));
    }
}

//***************************************************************************
void PluginManager::setSignalName(const QString &name)
{
    emit sigSignalNameChanged(name);
}

//***************************************************************************
void PluginManager::findPlugins()
{
    KStandardDirs dirs;

    QStringList files = dirs.findAllResources("module",
	    "plugins/kwave/*", KStandardDirs::NoDuplicates);

    /* fallback: search also in the old location (useful for developers) */
    files += dirs.findAllResources("data",
	    "kwave/plugins/*", KStandardDirs::NoDuplicates);

    foreach (QString file, files) {
	void *handle = dlopen(file.toLocal8Bit(), RTLD_NOW);
	if (handle) {
	    const char **name    = (const char **)dlsym(handle, "name");
	    const char **version = (const char **)dlsym(handle, "version");
	    const char **author  = (const char **)dlsym(handle, "author");

	    // skip it if something is missing or null
	    if (!name || !version || !author) continue;
	    if (!*name || !*version || !*author) continue;

	    KwaveSplash::showMessage(i18n("loading plugin %1...", *name));

	    m_plugin_files.insert(*name, file);
	    qDebug("%16s %5s written by %s",
		*name, *version, *author);

	    dlclose (handle);
	} else {
	    qWarning("error in '%s':\n\t %s",
		file.toLocal8Bit().data(), dlerror());
	}
    }

    qDebug("--- \n found %d plugins\n", m_plugin_files.count());
}

//***************************************************************************
void PluginManager::registerPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.append(factory);
}

//***************************************************************************
void PluginManager::unregisterPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.removeAll(factory);
}

//***************************************************************************
#include "PluginManager.moc"
//***************************************************************************
//***************************************************************************
