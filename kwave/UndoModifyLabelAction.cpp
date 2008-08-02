/***************************************************************************
 UndoAddLabelAction.cpp  -  Undo action for deleting labels
			     -------------------
    begin                : Sun Sep 03 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include <klocale.h>

#include "libkwave/Label.h"
#include "SignalWidget.h"
#include "UndoAddLabelAction.h"
#include "UndoModifyLabelAction.h"

//***************************************************************************
UndoModifyLabelAction::UndoModifyLabelAction(SignalWidget &signal_widget,
                                             Label &label)
    :UndoAction(), m_signal_widget(signal_widget), m_label(0),
     m_last_position(0)
{
    m_label = new Label(label);
    Q_ASSERT(m_label);
}

//***************************************************************************
UndoModifyLabelAction::~UndoModifyLabelAction()
{
    delete m_label;
}

//***************************************************************************
void UndoModifyLabelAction::setLastPosition(unsigned int pos)
{
    m_last_position = pos;
}

//***************************************************************************
QString UndoModifyLabelAction::description()
{
    return i18n("modify label");
}

//***************************************************************************
unsigned int UndoModifyLabelAction::undoSize()
{
    return sizeof(*this) + sizeof(Label);
}

//***************************************************************************
int UndoModifyLabelAction::redoSize()
{
    return sizeof(Label);
}

//***************************************************************************
void UndoModifyLabelAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
}

//***************************************************************************
UndoAction *UndoModifyLabelAction::undo(SignalManager &manager,
                                        bool with_redo)
{
    Q_ASSERT(m_label);
    if (!m_label) return 0;

//     qDebug("undo: last pos=%u, current pos=%u",
// 	   m_last_position, m_label->pos());

    Label *label = manager.findLabel(m_last_position);
    Q_ASSERT(label);
    if (!label) return 0;

    // store data for redo
    UndoModifyLabelAction *redo = 0;
    if (with_redo) {
	redo = new UndoModifyLabelAction(m_signal_widget, *label);
	Q_ASSERT(redo);
	if (redo) {
	    redo->setLastPosition(m_label->pos());
	    redo->store(manager);
	}
    }

    // modify the label
    label->moveTo(m_label->pos());
    label->rename(m_label->name());

    // redraw the markers layer
    m_signal_widget.refreshMarkersLayer();

    return redo;
}

//***************************************************************************
//***************************************************************************