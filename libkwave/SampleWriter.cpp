/***************************************************************************
  SampleWriter.cpp  -  stream for inserting samples into a track
			     -------------------
    begin                : Feb 11 2001
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

#include "mt/ThreadsafeX11Guard.h"
#include "libkwave/memcpy.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

/** size of m_buffer in samples */
#define BUFFER_SIZE (256*1024)

//***************************************************************************
SampleWriter::SampleWriter(Track &track, InsertMode mode,
    unsigned int left, unsigned int right)
    :Kwave::SampleSink(0,0),
     m_first(left), m_last(right), m_mode(mode), m_track(track),
     m_position(left),
     m_buffer(BUFFER_SIZE), m_buffer_used(0)
{
}

//***************************************************************************
SampleWriter::~SampleWriter()
{
    flush();
    Q_ASSERT(m_position <= m_last+1);

    // inform others that we proceeded
    emit sigSamplesWritten(m_position - m_first);
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const QMemArray<sample_t> &samples)
{
    unsigned int count = samples.size();

    if (m_buffer_used + count < m_buffer.size()) {
	// append to the internal buffer if there is still some room
	MEMCPY(&(m_buffer[m_buffer_used]), &(samples[0]),
	       count *sizeof(sample_t));
	m_buffer_used += count;
	if (m_buffer_used >= m_buffer.size()) flush();
    } else {
	// first flush the single-sample buffer before doing block operation
	if (m_buffer_used) flush();

	// now flush the block that we received as parameter (pass-through)
	flush(samples, count);
	Q_ASSERT(!count);
    }

    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const sample_t &sample)
{
    m_buffer[m_buffer_used++] = sample;
    if (m_buffer_used >= m_buffer.size()) flush();
    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (SampleReader &reader)
{
    if (m_buffer_used) flush();

    // transfer data, using our internal buffer
    unsigned int buflen = m_buffer.size();
    while (!reader.eof() && (m_position <= m_last)) {
	if (m_position+buflen-1 > m_last) buflen = (m_last-m_position)+1;

	m_buffer_used = reader.read(m_buffer, 0, buflen);
	Q_ASSERT(m_buffer_used);
	if (!m_buffer_used) break;

	flush();
    }

    // pad the rest with zeroes
    Q_ASSERT(m_position <= m_last+1);
    while (m_buffer_used + m_position <= m_last) {
	*this << static_cast<sample_t>(0);
	m_position++;
    }
    Q_ASSERT(m_position <= m_last+1);

    return *this;
}

//***************************************************************************
void SampleWriter::flush(const QMemArray<sample_t> &buffer,
                         unsigned int &count)
{
    if ((m_mode == Overwrite) && (m_position + count > m_last)) {
	// need clipping
	count = m_last + 1 - m_position;
// 	qDebug("SampleWriter::flush() clipped to count=%u", count);
    }

    if (count == 0) return; // nothing to flush

    m_track.writeSamples(m_mode, m_position, buffer, 0, count);
    m_position += count;
    if (m_position+1 > m_last) m_last = m_position-1;
    count = 0;

    // inform others that we proceeded
    emit proceeded();
}

//***************************************************************************
bool SampleWriter::eof()
{
    return (m_mode == Overwrite) ? (m_position > m_last) : false;
}

//***************************************************************************
SampleWriter &flush(SampleWriter &s)
{
    s.flush();
    return s;
}

//***************************************************************************
void SampleWriter::input(Kwave::SampleArray &data)
{
    ThreadsafeX11Guard x11_guard;
    if (data.size()) (*this) << data;
}

//***************************************************************************
#include "SampleWriter.moc"
//***************************************************************************
//***************************************************************************
