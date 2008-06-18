/***************************************************************************
    KwavePlaybackSink.h  -  multi-track Kwave compatible sink for playback
                             -------------------
    begin                : Sun Nov 04 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _KWAVE_PLAYBACK_SINK_H_
#define _KWAVE_PLAYBACK_SINK_H_

#include "config.h"

#include <QObject>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveSampleSink.h"

class PlayBackDevice;

namespace Kwave {

    class PlaybackSink: public Kwave::SampleSink
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param track index of this playback channel
	 */
	PlaybackSink(unsigned int track);

	/** Destructor */
	virtual ~PlaybackSink();

    signals:
	/** emits back the sample data received through input(...) */
	void output(unsigned int track, Kwave::SampleArray &data);

    public slots:
	/** receives sample data for this playback channel */
	void input(Kwave::SampleArray &data);

    private:
	/** index of the track of this playback channel */
	unsigned int m_track;
    };
}

#endif /* _KWAVE_PLAYBACK_SINK_H_ */
