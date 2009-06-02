/***************************************************************************
         SampleReader.h  -  stream for reading samples from a track
			     -------------------
    begin                : Apr 25 2001
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

#ifndef _SAMPLE_READER_H_
#define _SAMPLE_READER_H_

#include "config.h"

#include <QList>
#include <QObject>
#include <QTime>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/Sample.h"
#include "libkwave/Stripe.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveSampleSource.h"

class KDE_EXPORT SampleReader: public Kwave::SampleSource
{
    Q_OBJECT
public:

    /**
     * Constructor. Creates a stream for reading samples from a track.
     * @param mode the reader mode, see Kwave::ReaderMode
     * @param stripes list of stripes which contain data in the desired range
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleReader(Kwave::ReaderMode mode, QList<Stripe> stripes,
                 unsigned int left, unsigned int right);

    /** Destructor */
    virtual ~SampleReader();

    /** Resets the stream to it's start */
    void reset();

    /**
     * Each KwaveSampleSource has to derive this method for producing
     * sample data. It then should emit a signal like this:
     * "output(SampleArray &data)"
     */
    virtual void goOn();

    /** Checks if the last read operation has reached the end of input */
    inline bool eof() const {
	return (pos() > m_last);
    }

    /**
     * Returns true if the end of the source has been reached,
     * e.g. at EOF of an input stream.
     *
     * @return true if it can produce more sample data, otherwise false
     * @see eof()
     */
    virtual bool done() const { return eof(); }

    /**
     * Reads samples into a buffer.
     * @param buffer the destination buffer to receive the samples
     * @param dstoff offset within the destination buffer
     * @param length number of samples to read
     * @return number of read samples
     */
    unsigned int read(Kwave::SampleArray &buffer, unsigned int dstoff,
	unsigned int length);

    /**
     * Returns the minumum and maximum sample value within a range
     * of samples.
     * @param first index of the first sample
     * @param last index of the last sample
     * @param min receives the lowest value or 0 if no samples are in range
     * @param max receives the highest value or 0 if no samples are in range
     */
    void minMax(unsigned int first, unsigned int last,
		sample_t &min, sample_t &max);

    /** Skips a number of samples. */
    void skip(unsigned int count);

    /** Seeks to a given position */
    void seek(unsigned int pos);

    /**
     * Returns the current read position.
     */
    inline unsigned int pos() const {
	return (m_src_position + m_buffer_position - m_buffer_used);
    }

    /** Returns the position of the first sample */
    inline unsigned int first() const {
	return m_first;
    }

    /** Returns the position of the last sample */
    inline unsigned int last() const {
	return m_last;
    }

    /**
     * Reads one single sample.
     */
    SampleReader& operator >> (sample_t &sample);

    /**
     * Reads a full buffer of samples. If the buffer cannot be filled,
     * it will be shrinked to the number of samples that were really
     * read.
     */
    SampleReader& operator >> (Kwave::SampleArray &sample);

signals:

    /** Emitted when the internal buffer is filled or the reader is closed */
    void proceeded();

    /**
     * Interface for the signal/slot based streaming API.
     * @param data sample data that has been read
     */
    void output(Kwave::SampleArray data);

protected:

    /** Fills the sample buffer */
    void fillBuffer();

    /**
     * Read a block of samples, with padding if necessary.
     *
     * @param offset position where to start the read operation
     * @param buffer receives the samples
     * @param buf_offset offset within the buffer
     * @param length number of samples to read
     * @return number of read samples
     */
    unsigned int readSamples(unsigned int offset,
                             Kwave::SampleArray &buffer,
                             unsigned int buf_offset,
                             unsigned int length);

private:

    /** operation mode of the reader, see Kwave::ReaderMode */
    Kwave::ReaderMode m_mode;

    /** list of stipes in range */
    QList<Stripe> m_stripes;

    /**
     * Current sample position, related to the source of the samples. Does
     * not reflect the position of the next sample to be read out due to
     * internal buffering.
     * @see pos() for the output position
     */
    unsigned int m_src_position;

    /** first sample index */
    unsigned int m_first;

    /** last sample index */
    unsigned int m_last;

    /** intermediate buffer for the input data */
    Kwave::SampleArray m_buffer;

    /** number of used elements in the buffer */
    unsigned int m_buffer_used;

    /** read position within the buffer */
    unsigned int m_buffer_position;

    /** timer for limiting the number of progress signals per second */
    QTime m_progress_time;

};

#endif /* _SAMPLE_READER_H_ */
