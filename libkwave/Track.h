/***************************************************************************
                Track.h  -  collects one or more stripes in one track
			     -------------------
    begin                : Feb 09 2001
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

#ifndef _TRACK_H_
#define _TRACK_H_

#include "config.h"
#include <limits.h>  // for UINT_MAX

#include <QList>
#include <QObject>
#include <QReadWriteLock>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Stripe.h"

class SampleWriter;
class SampleReader;

//***************************************************************************
class KDE_EXPORT Track: public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor. Creates a new and empty track with
     * zero-length and no stripes
     */
    Track();

    /**
     * Constructor. Creates an empty track with one stripe
     * with specified length.
     */
    Track(unsigned int length);

    /**
     * Destructor.
     */
    virtual ~Track();

    /**
     * Returns the length of the track. This is equivalent
     * to the position of the last sample of the last Stripe.
     */
    unsigned int length();

    /**
     * Opens a stream for writing samples, starting at a
     * sample position.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleWriter *openSampleWriter(InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Opens a stream for reading samples. If the the last position
     * is omitted, the value UINT_MAX will be used.
     * @param mode read mode, see Kwave::ReaderMode
     * @param left first offset to be read (default = 0)
     * @param right last position to read (default = UINT_MAX)
     */
    SampleReader *openSampleReader(Kwave::ReaderMode mode,
	unsigned int left = 0,
	unsigned int right = UINT_MAX);

    /**
     * Deletes a range of samples
     * @param offset index of the first sample
     * @param length number of samples
     * @param make_gap if true, make a gap into the list of stripes
     *                 instead of moving the stuff from right to left
     */
    void deleteRange(unsigned int offset, unsigned int length,
                     bool make_gap = false);

    /** Returns the "selected" flag. */
    inline bool selected() const { return m_selected; }

    /** Sets the "selected" flag. */
    void select(bool select);

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

signals:

    /**
     * Emitted if the track has grown. This implies a modification of
     * the inserted data, so no extra sigSamplesModified is emitted.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see sigSamplesModified
     */
    void sigSamplesInserted(Track *src, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted if data has been removed from the track.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     */
    void sigSamplesDeleted(Track *src, unsigned int offset,
                           unsigned int length);

    /**
     * Emitted if some data within the track has been modified.
     * @param src source track of the signal (*this)
     * @param offset position from which the data was modified
     * @param length number of samples modified
     */
    void sigSamplesModified(Track *src, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted whenever the selection of the track has changed.
     * @param bool selected the current state of the selection
     */
     void sigSelectionChanged();

private:
    /**
     * Returns the current length of the stripe in samples. This
     * function uses no locks and is therefore reserved for internal
     * usage from within locked functions.
     * @note this must be private, it does no locking !
     */
    unsigned int unlockedLength();

    /**
     * Deletes a range of samples, used internally by deleteRange()
     * @param offset index of the first sample
     * @param length number of samples
     * @param make_gap if true, make a gap into the list of stripes
     *                 instead of moving the stuff from right to left
     */
    void unlockedDelete(unsigned int offset, unsigned int length,
                        bool make_gap = false);

    /**
     * Append samples after a given stripe.
     *
     * @param stripe the stripe after which to instert. Null pointer is
     *               allowed, in this case a new stripe is created
     * @param offset position where the new data should start
     * @param buffer array with samples
     * @param buf_offset offset within the buffer
     * @param length number of samples to write
     * @return true if successful, false if failed (e.g. out of memory)
     */
    bool appendAfter(Stripe *stripe, unsigned int offset,
                     const Kwave::SampleArray &buffer,
                     unsigned int buf_offset, unsigned int length);

    /**
     * Move all stripes after an offset to the right. Only looks at the
     * start position of the stripes, comparing with ">=", if the start
     * of a stripe is at the given offset, it will not be moved!
     *
     * @param offset position after which everything is moved right
     * @param length distance of the shift [samples]
     */
    void moveRight(unsigned int offset, unsigned int shift);

    /**
     * Append a new stripe with a given length.
     *
     * @param length number of samples, zero is allowed
     */
    Stripe *appendStripe(unsigned int length);

    /**
     * Split a stripe into two stripes. The new stripe will be created
     * from the right portion of the given stripe and the original
     * stripe will be shrinked to it's new size. The newly created stripe
     * will be inserted into m_stripes after the old one.
     *
     * @param stripe the stripe to be split
     * @param offset the offset within the stripe, which becomes the first
     *               sample in the new stripe
     * @return the new created stripe
     */
    Stripe *splitStripe(Stripe *stripe, unsigned int offset);

    /**
     * dump the list of stripes, for debugging
     * @internal for debugging only
     */
    void dump();

protected:

    friend class SampleWriter;

    /**
     * Write a block of samples. If necessary it starts, appends to,
     * or splits a stripe.
     *
     * @param mode a InsertMode (append/overwrite/insert)
     * @param offset position where to start the write operation
     * @param buffer array with samples
     * @param buf_offset offset within the buffer
     * @param length number of samples to write
     * @return true if successful, false if failed (e.g. out of memory)
     */
    bool writeSamples(InsertMode mode,
                      unsigned int offset,
                      const Kwave::SampleArray &buffer,
                      unsigned int buf_offset,
                      unsigned int length);

    friend class SampleReader;

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

    /** increments the usage counter (read lock to m_lock_usage) */
    void use();

    /** decrements the usage counter (read lock to m_lock_usage) */
    void release();

private:

    /**
     * Creates a new stripe with a start position and a length.
     * @param start offset of the first sample
     * @param length number of samples, zero is allowed
     * @note this must be private, it does no locking !
     */
    Stripe *newStripe(unsigned int start, unsigned int length);

private:
    /** read/write lock for access to the whole track */
    QReadWriteLock m_lock;

    /** lock to protect against deletion while the track is in use */
    QReadWriteLock m_lock_usage;

    /** list of stripes (a track actually is a container for stripes) */
    QList<Stripe *> m_stripes;

    /** True if the track is selected */
    bool m_selected;

};

#endif /* _TRACK_H_ */
