/*************************************************************************
    KwaveSampleSink.h  -  base class with a generic sample sink
                             -------------------
    begin                : Sun Oct 07 2007
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

#ifndef _KWAVE_SAMPLE_SINK_H_
#define _KWAVE_SAMPLE_SINK_H_

#include "config.h"
#include <qobject.h>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveStreamObject.h"

namespace Kwave {
    class SampleSink: public Kwave::StreamObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         *
         * @param parent a parent object, passed to QObject (optional)
         * @param name a free name, for identifying this object,
         *             will be passed to the QObject (optional)
         */
        SampleSink(QObject *parent=0, const char *name=0);

        /** Destructor */
        virtual ~SampleSink();

        /**
         * Returns true if the end of the destination has been reached,
         * e.g. EOF on the input stream, and it is unable to receive
         * more data.
         *
         * @return true if the sink can't receive more data, otherwise false
         */
        virtual bool done() { return false; };

    };
}

#endif /* _KWAVE_SAMPLE_SINK_H_ */