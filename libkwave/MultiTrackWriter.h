/***************************************************************************
      MultiTrackWriter.h - writer for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
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

#ifndef _MULTI_TRACK_WRITER_H_
#define _MULTI_TRACK_WRITER_H_

#include "config.h"
#include <qobject.h>
#include <qvector.h>
#include "SampleWriter.h"

class MultiTrackReader;

/**
 * A MultiTrackWriter encapsulates a set of <c>SampleWriter</c>s for
 * easier use of multi-track signals.
 */
class MultiTrackWriter: public QObject, private QVector<SampleWriter>
{
    Q_OBJECT
public:

    /** Constructor */
    MultiTrackWriter();

    /** Destructor */
    virtual ~MultiTrackWriter() {};

    /**
     * Transfers the content of multiple tracks into the destination.
     * Works the same way as the corresponding operator in
     * <c>SampleWriter</c>, but for multiple tracks. If the number of
     * tracks in source and destination do not match, the tracks will
     * be mixed up / down.
     */
    MultiTrackWriter &operator << (const MultiTrackReader &source);

    /** @see QVector::operator[] */
    inline virtual SampleWriter* operator[] (int i) const {
	return QVector<SampleWriter>::at(i);
    };

    /** @see QVector::count() */
    inline virtual unsigned int count() const {
	return QVector<SampleWriter>::count();
    };

    /** @see QVector::clear() */
    inline virtual void clear() { QVector<SampleWriter>::clear(); };

    /** @see QVector::isEmpty() */
    inline virtual bool isEmpty() {
        return QVector<SampleWriter>::isEmpty();
    };

    /** @see QVector::insert() */
    virtual bool insert(unsigned int track, const SampleWriter *writer);

    /** @see QVector::resize() */
    virtual bool resize(unsigned int size) {
        return QVector<SampleWriter>::resize(size);
    };

    /** returns true if the transfer has been cancelled */
    inline bool isCancelled() { return m_cancelled; };

signals:

    /**
     * Emits the current progress in totally processed samples, range is
     * from zero to the (length of the writer * number of tracks) - 1.
     */
    void progress(unsigned int samples);

public slots:

    /**
     * Can be connected to some progress dialog to cancel the current
     * transfer.
     */
    void cancel();

private slots:

    /**
     * Connected to each SampleWriter to get informed about their progress.
     */
    void proceeded();

protected:

    /**
     * Initialized as false, will be true if the transfer has
     * been cancelled
     */
    bool m_cancelled;

};

#endif /* _MULTI_TRACK_WRITER_H_ */
