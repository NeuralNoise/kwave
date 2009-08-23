/***************************************************************************
  AmplifyFreePlugin.cpp  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <QStringList>
#include <klocale.h>

#include "libkwave/KwaveConnect.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/KwaveMultiTrackSource.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/modules/CurveStreamAdapter.h"
#include "libkwave/modules/KwaveMul.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","2.1","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_action_name(""), m_params(), m_curve()
{
    i18n("amplifyfree");
}

//***************************************************************************
AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // store last parameters
    m_params = params;

    m_action_name = "";
    if (params.count() < 2) return -1;
    if (params.count() & 1) return -1; // no. of params must be even

    // first list entry == name of operation
    if (params[0].length()) m_action_name = params[0];

    // convert string list into command again...
    QString cmd;
    cmd = "curve(";
    for (int i = 1; i < params.count(); ++i) {
	cmd += params[i];
	if ((i + 1) < params.count()) cmd += ",";
    }
    cmd += ")";

    // and initialize our curve with it
    m_curve.fromCommand(cmd);

    return 0;
}

//***************************************************************************
QStringList *AmplifyFreePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    AmplifyFreeDialog *dialog = new AmplifyFreeDialog(parentWidget());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // remove the first list entry (action name), the rest is for the dialog
    if ((m_params.count() > 2) && !(m_params.count() & 1)) {
	QStringList curve_params = m_params;
	curve_params.takeFirst(); // ignore action name
	dialog->setParams(curve_params);
    }

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << "amplify free";
	QString cmd = dialog->getCommand();
	Parser p(cmd);
	while (!p.isDone()) *list << p.nextParam();

	qDebug("setup -> emitCommand('%s')",cmd.toLocal8Bit().data());
	emitCommand(cmd);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
QString AmplifyFreePlugin::progressText()
{
    return m_action_name.length() ?
	i18n(m_action_name.toLocal8Bit()) : i18n("amplify free");
}

//***************************************************************************
int AmplifyFreePlugin::start(QStringList &params)
{
    interpreteParameters(params);
    return Kwave::Plugin::start(params);
}

//***************************************************************************
void AmplifyFreePlugin::run(QStringList params)
{
    unsigned int first, last;
    QList<unsigned int> track_list;

    interpreteParameters(params);

    UndoTransactionGuard undo_guard(*this, i18n(m_action_name.toLocal8Bit()));

    unsigned int input_length = selection(&track_list, &first, &last, true);
    unsigned int tracks = track_list.count();

    // create all objects
    MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), selectedTracks(), first, last);
    Kwave::CurveStreamAdapter curve(m_curve, input_length);
    Kwave::MultiTrackWriter sink(signalManager(), track_list, Overwrite,
	first, last);
    Kwave::MultiTrackSource<Kwave::Mul, true> mul(tracks, this);

    // break if aborted
    if (!sink.tracks()) return;

    // connect them
    bool ok = true;
    if (ok) ok = Kwave::connect(
	source, SIGNAL(output(Kwave::SampleArray)),
	mul,    SLOT(input_a(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	curve,  SIGNAL(output(Kwave::SampleArray)),
	mul,    SLOT(input_b(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	mul,    SIGNAL(output(Kwave::SampleArray)),
	sink,   SLOT(input(Kwave::SampleArray)));
    if (!ok) {
	return;
    }

    // connect the progress dialog
    connect(&sink, SIGNAL(progress(unsigned int)),
	    this,  SLOT(updateProgress(unsigned int)),
	    Qt::BlockingQueuedConnection);

    // transport the samples
    qDebug("AmplifyFreePlugin: filter started...");
    while (!shouldStop() && !source.done()) {
	source.goOn();
	curve.goOn();
	/* mul.goOn(); */
    }
    qDebug("AmplifyFreePlugin: filter done.");
}

//***************************************************************************
#include "AmplifyFreePlugin.moc"
//***************************************************************************
//***************************************************************************
