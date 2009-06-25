/***************************************************************************
    UndoReverseAction.h  -  undo action for the "reverse" effect
			     -------------------
    begin                : Wed Jun 24 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _UNDO_REVERSE_ACTION_H_
#define _UNDO_REVERSE_ACTION_H_

#include "config.h"

#include <QString>
#include "libkwave/undo/UndoAction.h"

class SignalManager;
namespace Kwave { class PluginManager; }

class UndoReverseAction: public UndoAction
{
public:

    /** Constructor */
    UndoReverseAction(Kwave::PluginManager &plugin_manager);

    /** Destructor */
    virtual ~UndoReverseAction();

    /**
     * Returns a verbose short description of the action.
     */
    virtual QString description();

    /**
     * Returns the required amount of memory that is needed for storing
     * undo data for the operation. This will be called to determine the
     * free memory to be reserved.
     * @note this is the first step (after the constructor)
     */
    virtual unsigned int undoSize();

    /**
     * Returns the difference of needed memory that is needed for
     * redo.
     */
    virtual int redoSize();

    /**
     * Stores the data needed for undo.
     * @param manager the SignalManager for modifying the signal
     * @note this is the second step, after size() has been called
     * @return true if successful, false if failed (e.g. out of memory)
     */
    virtual bool store(SignalManager &manager);

    /**
     * Takes back an action by creating a new undo action (for further
     * redo) and restoring the previous state.
     * @param manager the SignalManager for modifying the signal
     * @param with_redo if true a UndoAction for redo will be created
     * @note The return value is allowed to be the same object. This
     *       is useful for objects that can re-use their data for
     *       undo/redo. You have to check for this when deleting an
     *       UndoAction object after undo.
     */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

private:

    /** plugin manager, for emitting the sigCommand(reverse()) */
    Kwave::PluginManager &m_plugin_manager;

};

#endif /* _UNDO_REVERSE_ACTION_H_ */
