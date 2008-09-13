/***************************************************************************
     UndoModifyAction.h  -  UndoAction for modifications on samples
			     -------------------
    begin                : May 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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
#include <klocale.h>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoModifyAction.h"

/** size of the buffer for internal copy operations */
#define BUFFER_SIZE 65536

//***************************************************************************
UndoModifyAction::UndoModifyAction(unsigned int track, unsigned int offset,
                                   unsigned int length)
    :UndoAction(), m_track(track), m_offset(offset), m_length(length),
     m_buffer_track()
{
}

//***************************************************************************
UndoModifyAction::~UndoModifyAction()
{
}

//***************************************************************************
QString UndoModifyAction::description()
{
    return i18n("modify samples");
}

//***************************************************************************
unsigned int UndoModifyAction::undoSize()
{
    return sizeof(*this) + (m_length * sizeof(sample_t));
}

//***************************************************************************
bool UndoModifyAction::store(SignalManager &manager)
{
    SampleReader *reader = manager.openSampleReader(
	m_track, m_offset, m_offset+m_length-1);
    Q_ASSERT(reader);
    if (!reader) return false;

    SampleWriter *writer = m_buffer_track.openSampleWriter(
        Append, 0, m_length-1);
    Q_ASSERT(writer);
    if (!writer) {
	delete reader;
	return false;
    }

    // copy the data
    (*writer) << (*reader);

    delete reader;
    delete writer;
    return (m_buffer_track.length() == m_length);
}

//***************************************************************************
UndoAction *UndoModifyAction::undo(SignalManager &manager, bool with_redo)
{
    SampleWriter *writer = manager.openSampleWriter(
	m_track, Overwrite, m_offset, m_offset+m_length-1);
    Q_ASSERT(writer);
    if (!writer) return 0;

    unsigned int len = m_length;

    if (with_redo) {
	Kwave::SampleArray buf_cur(BUFFER_SIZE);
	Kwave::SampleArray buf_sav(BUFFER_SIZE);

	SampleReader *reader_cur = manager.openSampleReader(
	    m_track, m_offset, m_offset+m_length-1);
	SampleWriter *writer_cur = writer;
	SampleReader *reader_sav = m_buffer_track.openSampleReader(
	    0, m_length-1);
	SampleWriter *writer_sav = m_buffer_track.openSampleWriter(
	    Overwrite, 0, m_length-1);

	// exchange content of the current signal with the content
	// of the internal buffer
	while (reader_cur && reader_sav && writer_sav && len) {
	    // 1. fill buf_cur with data from current signal
	    (*reader_cur) >> buf_cur;
	    Q_ASSERT(buf_cur.size());
	    if (!buf_cur.size()) break;

	    // 2. fill buf_sav with data from buffer
	    (*reader_sav) >> buf_sav;
	    Q_ASSERT(buf_sav.size() == buf_cur.size());
	    if (buf_sav.size() != buf_cur.size()) break;

	    // 3. write buf_cur to buffer
	    (*writer_sav) << buf_cur;

	    // 4. write buf_sav to current signal
	    (*writer_cur) << buf_sav;

	    len = (len > buf_cur.size()) ? (len - buf_cur.size()) : 0;
	}
	Q_ASSERT(m_buffer_track.length() == m_length);

	if (reader_cur) delete reader_cur;
	if (reader_sav) delete reader_sav;
	if (writer_sav) delete writer_sav;
    } else {
	SampleReader *reader = m_buffer_track.openSampleReader(
	    0, m_length-1);
	Q_ASSERT(reader);

	if (reader && writer) (*writer) << (*reader);
    }

    delete writer;
    return (with_redo) ? this : 0;
}

//***************************************************************************
//***************************************************************************