/*************************************************************************
    HMSTimeWidget.cpp  -  widget for setting a time in hours, minutes, seconds
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

#include "config.h"
#include <limits.h>
#include <knuminput.h>
#include "HMSTimeWidget.h"

//***************************************************************************
HMSTimeWidget::HMSTimeWidget(QWidget *parent, const char *name)
    :HMSTimeWidgetBase(parent, name),
     m_time(0), m_limit(INT_MAX)
{
    setValue(m_time);
    connect();
}

//***************************************************************************
HMSTimeWidget::~HMSTimeWidget()
{
}

//***************************************************************************
int HMSTimeWidget::value()
{
    return m_time;
}

//***************************************************************************
void HMSTimeWidget::setValue(int value)
{
    if (value < 0) value = 0;
    if ((unsigned int)value > m_limit) value = m_limit;
    m_time = value;

    const int seconds = (m_time % 60);
    const int minutes = (m_time / 60) % 60;
    const int hours   = (m_time / (60*60));

    sbHours->setValue(hours);
    sbMinutes->setValue(minutes);
    sbSeconds->setValue(seconds);
}

//***************************************************************************
void HMSTimeWidget::setLimit(unsigned int limit)
{
    Q_ASSERT(limit <= INT_MAX);
    if (limit > INT_MAX) limit = INT_MAX;
    if (limit < m_limit) {
	m_limit = limit;
	setValue(m_limit);
    } else {
	m_limit = limit;
    }
}

//***************************************************************************
void HMSTimeWidget::timeChanged(int)
{
    // get current time and correct wrap-overs
    int seconds = sbSeconds->value();
    int minutes = sbMinutes->value();
    int hours   = sbHours->value();

    if (seconds < 0) {
	seconds = 59;
	minutes--;
    }
    if (minutes < 0) {
	minutes = 59;
	hours--;
    }
    if (hours < 0) {
	hours = 0;
	minutes = 0;
	seconds = 0;
    }

    Q_ASSERT((hours >= 0) && (minutes >= 0) && (seconds >= 0));
    unsigned int time = seconds + (minutes + (hours * 60L)) * 60L;

    bool changed = (time != m_time);

    disconnect();
    setValue(time);
    connect();

    if (changed) emit valueChanged(m_time); // emit the change
}

//***************************************************************************
void HMSTimeWidget::connect()
{
    QObject::connect(sbSeconds, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
    QObject::connect(sbMinutes, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
    QObject::connect(sbHours, SIGNAL(valueChanged(int)),
                     this, SLOT(timeChanged(int)));
}

//***************************************************************************
void HMSTimeWidget::disconnect()
{
    // disconnect the time controls
    QObject::disconnect(sbSeconds, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
    QObject::disconnect(sbMinutes, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
    QObject::disconnect(sbHours, SIGNAL(valueChanged(int)),
                        this, SLOT(timeChanged(int)));
}

//***************************************************************************
//***************************************************************************