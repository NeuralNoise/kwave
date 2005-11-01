/***************************************************************************
  ArtsMultiSink.h  -  base class for multi-track aRts compatible sinks
                             -------------------
    begin                : Sun Dec 9 2001
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

#ifndef _ARTS_MULTI_SINK_H_
#define _ARTS_MULTI_SINK_H_

#include "config.h"
#include <qglobal.h> // for qFatal(...)
#include <arts/artsflow.h>

class ArtsMultiSink
{
public:

    /** Destructor */
    virtual ~ArtsMultiSink() {};

    /**
     * Returns a pointer to one of the aRts sample sinks. (Used for
     * connecting to the internal aRts streams)
     * @param i index of the track [0...count-1]
     * @return pointer to an Arts::Object
     */
    virtual Arts::Object *operator [] (unsigned int i) = 0;

    /**
     * Can be overwritten in a derived class that is used as the
     * last sink in a chain of aRts (compatible) objects to
     * start/continue the data processing.
     */
    virtual void goOn() {
	qFatal("ArtsMultiSink::goOn(): SHOULD NEVER BE CALLED!");
    };

    /**
     * Can be overwritten in a derived class that is used as the
     * last sink in a chain of aRts (compatible) objects to
     * determine the end of data processing (sink is full/eof detection).
     */
    virtual bool done() {
	qFatal("ArtsMultiSink::done(): SHOULD NEVER BE CALLED!");
	return true;
    };

    /** @see Arts::StdSynthModule::start() */
    virtual void start() {};

    /** @see Arts::StdSynthModule::stop() */
    virtual void stop() {};

};

#endif /* _ARTS_MULTI_SINK_H_ */
