/*************************************************************************
    HMSTimeWidget.h  -  widget for setting a time in hours, minutes, seconds
                             -------------------
    begin                : Sat Sep 06 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _HMS_TIME_WIDGET_H_
#define _HMS_TIME_WIDGET_H_

#include "config.h"
#include <qobject.h>
#include <qwidget.h>
#include "libgui/HMSTimeWidgetBase.h"

class HMSTimeWidget: public HMSTimeWidgetBase
{
    Q_OBJECT
public:

    /** Constructor */
    HMSTimeWidget(QWidget *parent, const char *name);

    /** Destructor */
    virtual ~HMSTimeWidget();

    /** get the time as a number of seconds */
    virtual int value();

signals:

    /** emitted when the time value has changed */
    void valueChanged(int value);

public slots:

    /** set the time, given as a number in seconds */
    virtual void setValue(int value);

    /** sets the maximum time in seconds */
    virtual void setLimit(unsigned int limit);

protected slots:

    /** called whenever one of the time controls has changed */
    void timeChanged(int);

private:

    /** connect all signals to avoid loops @internal */
    void connect();

    /** disconnect all signals to avoid loops @internal */
    void disconnect();

private:

    /** the currently selected time in seconds */
    unsigned int m_time;

    /** the maximum time in seconds, for limiting m_time */
    unsigned int m_limit;

};

#endif /* _HMS_TIME_WIDGET_H_ */
