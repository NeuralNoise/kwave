/***************************************************************************
            TopWidget.h  -  Toplevel widget of Kwave
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TOP_WIDGET_H_
#define _TOP_WIDGET_H_

#include "config.h"
#include <qcstring.h>
#include <qstringlist.h>
#include <kmainwindow.h>
#include <kurl.h>

class QCloseEvent;
class QTimer;

class KCombo;
class KDNDDropZone;
class KToolBar;
class KStatusBar;

class KwaveApp;
class MenuManager;
class MainWidget;
class PluginManager;
class SignalManager;

/**
 * Toplevel widget of the Kwave application. Holds a main widget, a menu
 * bar, a status bar and a toolbar.
 */
class TopWidget : public KMainWindow
{
    Q_OBJECT

public:

    /**
     * Constructor. Creates a new toplevel widget including menu bar,
     * buttons, working are an s on.
     * @param main_app reference to the main kwave aplication object
     */
    TopWidget(KwaveApp &main_app);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /**
     * Destructor.
     */
    ~TopWidget();

    /**
     * Loads a new file and updates the widget's title, menu, status bar
     * and so on.
     * @param url URL of the file to be loaded
     * @return 0 if successful
     */
    int loadFile(const KURL &url);

    /**
     * Returns the reference to the Kwave application
     */
    inline KwaveApp &getKwaveApp() { return m_app; };

    /**
     * Returns a reference to the current name of the signal. If no signal is
     * loaded the string is zero-length.
     */
    QString signalName();

    /**
     * Parses a buffer (that is intended to contain the content
     * of a text file) into lines with commands and executes all
     * parsed commands.
     * @param buffer the list of commands
     */
    int parseCommands(const QByteArray &buffer);

    /**
     * Loads a batch file into memory, parses and executes
     * all commands in it.
     * @param filename name of the batch file
     */
    int loadBatch(const QString &filename);

    /**
     * Returns a pointer to the current signal manager.
     */
    SignalManager &signalManager();

public slots:

    int executeCommand(const QString &command);

    /**
     * Updates the list of recent files in the menu, maybe some other
     * window has changed it. The list of recent files is static and
     * global in KwaveApp.
     */
    void updateRecentFiles();

protected slots:

    /** @see QWidget::closeEvent() */
    virtual void closeEvent(QCloseEvent *e);

private slots:

    /** called on changes in the zoom selection combo box */
    void selectZoom(int index);

    /**
     * Called if a new zoom factor has been set in order to update
     * the status display and the content of the zomm selection
     * combo box.
     * @note This method can not be called to *set* a new zoom factor.
     */
    void setZoomInfo(double zoom);

    /**
     * Called if the status information of the signal has been changed
     * or become valid.
     * @param length number of samples
     * @param tracks number of tracks
     * @param rate sample rate [samples/second]
     * @param bits resolution in bits
     */
    void setStatusInfo(unsigned int length, unsigned int tracks,
                       unsigned int rate, unsigned int bits);

    /**
     * Called if the number of tracks has changed and updates
     * the menu.
     */
    void setTrackInfo(unsigned int tracks);

    /**
     * Updates the number of selected samples in the status bar.
     * @param samples number of selected samples
     * @param ms length of the selected range [milliseconds]
     */
    void setSelectedTimeInfo(unsigned int samples, double ms);

    /**
     * Sets the descriptions of the last undo and redo action. If the
     * name is zero or zero-length, the undo / redo is currently not
     * available.
     */
    void setUndoRedoInfo(const QString &undo, const QString &redo);

    /**
     * Updates the status bar's content depending on the current status
     * or position of the mouse cursor.
     */
    void mouseChanged(int mode);

    /** updates all elements in the toolbar */
    void updateToolbar();

    /** updates only the playback controls */
    void updatePlaybackControls();

    /** called if the playback has been paused */
    void playbackPaused();

    /** connected to the clicked() signal of the pause button */
    void pausePressed();

    /** toggles the state of the pause button */
    void blinkPause();

    /** toolbar: "file/new" */
    void toolbarFileNew()    { executeCommand("plugin(newsignal)"); };

    /** toolbar: "file/open" */
    void toolbarFileOpen()   { executeCommand("open () "); };

    /** toolbar: "file/save" */
    void toolbarFileSave()   { executeCommand("save () "); };

    /** toolbar: "edit/undo" */
    void toolbarEditUndo()   { executeCommand("undo () "); };

    /** toolbar: "edit/redo" */
    void toolbarEditRedo()   { executeCommand("redo () "); };

    /** toolbar: "edit/cut" */
    void toolbarEditCut()    { executeCommand("cut () "); };

    /** toolbar: "edit/copy" */
    void toolbarEditCopy()   { executeCommand("copy () "); };

    /** toolbar: "edit/paste" */
    void toolbarEditPaste()  { executeCommand("paste () "); };

    /** toolbar: "edit/erase" */
    void toolbarEditErase()  { executeCommand("plugin(zero)"); };

    /** toolbar: "edit/delete" */
    void toolbarEditDelete() { executeCommand("delete () "); };

    /** called if the signal now or no longer is modified */
    void modifiedChanged(bool);

signals:
    /**
     * Tells this TopWidget's parent to execute a command
     */
    void sigCommand(const QString &command);

    /**
     * Emitted it the name of the signal has changed.
     */
    void sigSignalNameChanged(const QString &name);

protected:

    /** Updates the menu by enabling/disabling some entries */
    void updateMenu();

    /**
     * Discards all changes to the current file and loads
     * it again.
     */
    int revert();

    /**
     * Shows an "open file" dialog and opens the .wav file the
     * user has selected.
     */
    int openFile();

    /**
     * Closes the current file and updates the menu and other controls.
     * If the file has been modified and the user wanted to break
     * the close operation, the file will not get closed and the
     * function returns with false.
     * @return true if closing is allowed
     */
    bool closeFile();

    /** Saves the current file. */
    int saveFile();

    /**
     * Opens a dialog for saving the current .wav file.
     * @param selection if set to true, only the current selection
     *        will be saved
     */
    int saveFileAs(bool selection = false);

    /**
     * Closes the current file and creates a new empty signal.
     * @param samples number of samples per track
     * @param rate sample rate
     * @param bits number of bits per sample
     * @param tracks number of tracks
     */
    int newSignal(unsigned int samples, double rate,
                  unsigned int bits, unsigned int tracks);

    /**
     * Opens a file contained in the list of recent files.
     * @param str the entry contained in the list
     */
    int openRecent(const QString &str);

    /**
     * Sets a new resolution for saving in bits per sample.
     * @param str str the resolution in string representation
     */
    int resolution(const QString &str);

    /** Updates the caption with the filename */
    void updateCaption();

    /** handle playback commands, like play/stop/pause etc... */
    int executePlaybackCommand(const QString &command);

private:

    /**
     * Primitive class that holds a list of predefined zoom
     * factors.
     */
    class ZoomListPrivate: public QStringList
    {
    public:
	ZoomListPrivate();
	virtual ~ZoomListPrivate() {};
	unsigned int ms(int index);
    private:
	void append(const char *text, unsigned int ms);
    private:
	QValueList<unsigned int> m_milliseconds;
    };

    /** Initialized list of zoom factors */
    ZoomListPrivate m_zoom_factors;

    /** reference to the main kwave application */
    KwaveApp &m_app;

    /** our internal plugin manager */
    PluginManager *m_plugin_manager;

    /**
     * the main widget with all views and controls (except menu and
     * toolbar)
     */
    MainWidget *m_main_widget;

    /** combo box for selection of the zoom factor */
    KComboBox *m_zoomselect;

    /**
     * toolbar for controlling file operations, copy&paste,
     * playback and zoom
     */
    KToolBar *m_toolbar;

    /** URL of the current signal or file. Empty if nothing loaded */
    KURL m_url;

    /** menu manager for this window */
    MenuManager *m_menu_manager;

    /** bits per sample to save with */
    int m_save_bits;

    /** Timer used to let the pause button blink... */
    QTimer *m_pause_timer;

    /** determines the state of blinking toolbar buttons */
    bool m_blink_on;

    /** member id of the "edit undo" toolbar button */
    int m_id_undo;

    /** member id of the "edit redo" toolbar button */
    int m_id_redo;

    /** member id of the "start playback" toolbar button */
    int m_id_play;

    /** member id of the "start playback and loop" toolbar button */
    int m_id_loop;

    /** member id of the "pause playback" toolbar button */
    int m_id_pause;

    /** member id of the "stop playback" toolbar button */
    int m_id_stop;

    /** member id of the "zoom to selection" toolbar button */
    int m_id_zoomselection;

    /** member id of the "zoom in" toolbar button */
    int m_id_zoomin;

    /** member id of the "zoom out" toolbar button */
    int m_id_zoomout;

    /** member id of the "zoom to 100%" toolbar button */
    int m_id_zoomnormal;

    /** member id of the "zoom to all" toolbar button */
    int m_id_zoomall;

    /** member id of the "zoom factor" combobox in the toolbar */
    int m_id_zoomselect;

};

#endif /* _TOP_WIDGET_H_ */
