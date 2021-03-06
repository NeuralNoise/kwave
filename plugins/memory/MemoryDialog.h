/***************************************************************************
         MemoryDialog.h  -  setup dialog of Kwave's memory management
                             -------------------
    begin                : Sun Aug 05 2001
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

#ifndef MEMORY_DIALOG_H
#define MEMORY_DIALOG_H

#include "config.h"
#include "ui_MemDlg.h"
#include <QDialog>
#include <QObject>

class QStringList;

namespace Kwave
{
    class MemoryDialog :public QDialog, public Ui::MemDlg
    {
	Q_OBJECT

    public:
	/**
	 * Constructor.
	 * @param parent the dialog's parent widget
	 * @param physical_limited determines if the physical memory is limited
	 * @param physical_limit limit of physical memory [megabytes], 0=no limit
	 * @param virtual_enabled true if virtual memory is enabled
	 * @param virtual_limited determines if the virtual memory is limited
	 * @param virtual_limit limit of virtual memory [megabytes],
	 *        0=disabled, UINT_MAX = no limit
	 * @param virtual_dir directory for virtual memory files
	 * @param undo_limit limit of memory for undo/redo [megabytes]
	 */
	MemoryDialog(QWidget* parent, bool physical_limited,
	    unsigned int physical_limit, bool virtual_enabled,
	    bool virtual_limited, unsigned int virtual_limit,
	    const QString &virtual_dir, unsigned int undo_limit);

	/** Returns true if the dialog is usable (no null pointers) */
	bool isOK();

	/** Destructor */
	virtual ~MemoryDialog();

	/** Returns all parameters as a list. */
	void params(QStringList &par);

    protected slots:

	/** Called if the virtual memory has been enabled / disabled */
	void virtualMemoryEnabled(bool enable);

	/** Connected to the Search button to select a new swap directory */
	void searchSwapDir();

	/** invoke the online help */
	void invokeHelp();

    };
}

#endif /* MEMORY_DIALOG_H */

//***************************************************************************
//***************************************************************************
