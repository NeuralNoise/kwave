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

#include <QtCore/QPair>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>

#include <kdemacros.h>
#include <kmainwindow.h>
#include <kurl.h>

#include "libkwave/FileContext.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"

#include "libgui/MouseMark.h"

class QCloseEvent;
class QLabel;
class QTextStream;
class QTimer;

class KCombo;
class KComboBox;
class KDNDDropZone;
class KStatusBar;

namespace Kwave
{

    class App;
    class FileContext;
    class MainWidget;
    class MenuManager;
    class SignalManager;
    class PlayerToolBar;
    class PluginManager;
    class FileContext;
    class ZoomToolBar;

    /**
     * Toplevel widget of the Kwave application. Holds a main widget, a menu
     * bar, a status bar and a toolbar.
     */
    class KDE_EXPORT TopWidget : public KMainWindow
    {
	Q_OBJECT

    public:

	/**
	 * Constructor. Creates a new toplevel widget including menu bar,
	 * buttons, working are an s on.
	 * @param app reference to the Kwave application instance
	 */
	TopWidget(Kwave::App &app);

	/**
	 * Does some initialization at startup of the instance
	 *
	 * @retval true if this instance was successfully initialized
	 * @retval false if something went wrong during initialization
	 */
	bool init();

	/**
	 * Destructor.
	 */
	virtual ~TopWidget();

	/**
	 * Loads a new file and updates the widget's title, menu, status bar
	 * and so on.
	 * @param url URL of the file to be loaded
	 * @return 0 if successful
	 */
	int loadFile(const KUrl &url);

    public slots:

	/**
	 * Updates the list of recent files in the menu, maybe some other
	 * window has changed it. The list of recent files is static and
	 * global in KwaveApp.
	 */
	void updateRecentFiles();

	/**
	 * Execute a Kwave text command
	 * @param command a text command
	 * @retval 0 if succeeded
	 * @retval negative error code if failed
	 * @retval ENOSYS if the command is unknown in this component
	 */
	int executeCommand(const QString &command);

	/**
	 * forward a Kwave text command coming from an upper layer to
	 * the currently active context below us (which is the main
	 * entry point for all text commands)
	 * @param command a text command
	 */
	void forwardCommand(const QString &command);

    protected slots:

	/** @see QWidget::closeEvent() */
	virtual void closeEvent(QCloseEvent *e);

    private slots:

	/**
	 * Called when the meta data of the current signal has changed
	 * @param meta_data the new meta data, after the change
	 */
	void metaDataChanged(Kwave::MetaDataList meta_data);

	/**
	 * Updates the number of selected samples in the status bar.
	 * @param offset index of the first selected sample
	 * @param length number of selected samples
	 */
	void selectionChanged(sample_index_t offset, sample_index_t length);

	/**
	 * Sets the descriptions of the last undo and redo action. If the
	 * name is zero or zero-length, the undo / redo is currently not
	 * available.
	 */
	void setUndoRedoInfo(const QString &undo, const QString &redo);

	/**
	 * Updates the status bar's content depending on the current status
	 * or position of the mouse cursor.
	 * @param mode one of the modes in enum MouseMode
	 * @param offset selection start (not valid if mode is MouseNormal)
	 * @param length selection length (not valid if mode is MouseNormal)
	 */
	void mouseChanged(Kwave::MouseMark::Mode mode,
	                  sample_index_t offset, sample_index_t length);

	/** updates the menus when the clipboard has become empty/full */
	void clipboardChanged(bool data_available);

	/** Updates the menu by enabling/disabling some entries */
	void updateMenu();

	/** resets the toolbar layout to default settings */
	void resetToolbarToDefaults();

	/** updates all elements in the toolbar */
	void updateToolbar();

	void toolbarRecord() {
	    forwardCommand(_("plugin(record)"));
	}

	/** toolbar: "file/new" */
	void toolbarFileNew() {
	    forwardCommand(_("plugin(newsignal)"));
	}

	/** toolbar: "file/open" */
	void toolbarFileOpen() {
	    forwardCommand(_("open () "));
	}

	/** toolbar: "file/save" */
	void toolbarFileSave() {
	    forwardCommand(_("save () "));
	}

	/** toolbar: "file/save" */
	void toolbarFileSaveAs() {
	    forwardCommand(_("saveas () "));
	}

	/** toolbar: "file/save" */
	void toolbarFileClose() {
	    forwardCommand(_("close () "));
	}

	/** toolbar: "edit/undo" */
	void toolbarEditUndo() {
	    forwardCommand(_("undo () "));
	}

	/** toolbar: "edit/redo" */
	void toolbarEditRedo() {
	    forwardCommand(_("redo () "));
	}

	/** toolbar: "edit/cut" */
	void toolbarEditCut() {
	    forwardCommand(_("cut () "));
	}

	/** toolbar: "edit/copy" */
	void toolbarEditCopy() {
	    forwardCommand(_("copy () "));
	}

	/** toolbar: "edit/paste" */
	void toolbarEditPaste() {
	    forwardCommand(_("paste () "));
	}

	/** toolbar: "edit/erase" */
	void toolbarEditErase() {
	    forwardCommand(_("plugin(zero)"));
	}

	/** toolbar: "edit/delete" */
	void toolbarEditDelete() {
	    forwardCommand(_("delete () "));
	}

	/**
	 * called if the signal now or no longer is modified
	 * @param modified if true: signal now is "modified", otherwise not
	 */
	void modifiedChanged(bool modified);

	/** shows a message/progress in the splash screen */
	void showInSplashSreen(const QString &message);

	/**
	 * Show a message in the status bar
	 * @param msg the status bar message, already localized
	 * @param ms the time in milliseconds to show the message
	 */
	void showStatusBarMessage(const QString &msg, unsigned int ms);

    signals:

	/**
	 * Emitted by us when the current file context has switched
	 * @param context the new file context
	 */
	void sigFileContextSwitched(Kwave::FileContext *context);

	/**
	 * Emitted it the name of the signal has changed.
	 */
	void sigSignalNameChanged(const QString &name);

    private:

	/**
	 * Closes the current file and creates a new empty signal.
	 * @param samples number of samples per track
	 * @param rate sample rate
	 * @param bits number of bits per sample
	 * @param tracks number of tracks
	 * @return zero if successful, -1 if failed or canceled
	 */
	int newSignal(sample_index_t samples, double rate,
	              unsigned int bits, unsigned int tracks);

	/**
	 * Discards all changes to the current file and loads
	 * it again.
	 * @return zero if succeeded or error code
	 */
	int revert();

	/**
	 * Shows an "open file" dialog and opens the .wav file the
	 * user has selected.
	 * @return zero if succeeded or error code
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

	/**
	 * Saves the current file.
	 * @return zero if succeeded, non-zero if failed
	 */
	int saveFile();

	/**
	 * Opens a dialog for saving the current .wav file.
	 * @param filename the name of the new file
	 *                 or empty string to open the File/SaveAs dialog
	 * @param selection if set to true, only the current selection
	 *        will be saved
	 * @return zero if succeeded, non-zero if failed
	 */
	int saveFileAs(const QString &filename, bool selection = false);

	/**
	 * Opens a file contained in the list of recent files.
	 * @param str the entry contained in the list
	 * @return zero if succeeded, non-zero if failed
	 */
	int openRecent(const QString &str);

	/**
	 * Updates the caption with the filename
	 * @param name the filename to show in the caption
	 * @param is_modified true if the file is modified, false if not
	 */
	void updateCaption(const QString &name, bool is_modified);

	/** returns the name of the signal */
	QString signalName() const;

	/**
	 * Creates a new file context and initializes it.
	 * @retval true if succeeded
	 * @retval false if creation or initialization failed
	 */
	bool newFileContext();

    private:

	/** each TopWidget has exactly one corresponding Kwave::App instance */
	Kwave::App &m_application;

	/** application context of this instance */
	QPointer<Kwave::FileContext> m_current_context;

	/** toolbar with playback/record and seek controls */
	Kwave::PlayerToolBar *m_toolbar_record_playback;

	/** toolbar with zoom controls */
	Kwave::ZoomToolBar *m_toolbar_zoom;

	/** menu manager for this window */
	Kwave::MenuManager *m_menu_manager;

// 	/** MDI area, parent of all MDI child windows */
// 	QMdiArea *m_mdi_area;

	/** action of the "file save" toolbar button */
	QAction *m_action_save;

	/** action of the "file save as..." toolbar button */
	QAction *m_action_save_as;

	/** action of the "file close" toolbar button */
	QAction *m_action_close;

	/** action of the "edit undo" toolbar button */
	QAction *m_action_undo;

	/** action of the "edit redo" toolbar button */
	QAction *m_action_redo;

	/** status bar label for length of the signal */
	QLabel *m_lbl_status_size;

	/** status bar label for mode information */
	QLabel *m_lbl_status_mode;

	/** status bar label for cursor / playback position */
	QLabel *m_lbl_status_cursor;
    };
}

#endif /* _TOP_WIDGET_H_ */

//***************************************************************************
//***************************************************************************
