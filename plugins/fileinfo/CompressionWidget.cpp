/***************************************************************************
  CompressionWidget.cpp  -  widget for setting ogg or mp3 compression rates
                             -------------------
    begin                : Sat Jun 14 2003
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <qobject.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qvaluelist.h>
#include <qwhatsthis.h>
#include <knuminput.h>

#include "BitrateWidget.h"
#include "CompressionWidget.h"

//***************************************************************************
CompressionWidget::CompressionWidget(QWidget *parent, const char *name)
    :CompressionWidgetBase(parent, name)
{

    // use well-known bitrates from MP3
    QValueList<int> rates;
    rates.append(  8000);
    rates.append( 16000);
    rates.append( 24000);
    rates.append( 32000);
    rates.append( 40000);
    rates.append( 56000);
    rates.append( 64000);
    rates.append( 80000);
    rates.append( 96000);
    rates.append(112000);
    rates.append(128000);
    rates.append(144000);
    rates.append(160000);
    rates.append(176000);
    rates.append(192000);
    rates.append(224000);
    rates.append(256000);
    rates.append(288000);
    rates.append(320000);
    rates.append(352000);
    rates.append(384000);
    rates.append(416000);
    rates.append(448000);
    abrBitrate->allowRates(rates);
    abrHighestBitrate->allowRates(rates);
    abrLowestBitrate->allowRates(rates);
    
//    abrHighestBitrate->setSpecialValueText(i18n("no limit"));
//    abrLowestBitrate->setSpecialValueText(i18n("no limit"));

    connect(rbABR, SIGNAL(toggled(bool)),
            this,  SLOT(selectABR(bool)));
    connect(chkLowestBitrate, SIGNAL(toggled(bool)),
            this,  SLOT(lowestToggled(bool)));
    connect(chkHighestBitrate, SIGNAL(toggled(bool)),
            this,  SLOT(highestToggled(bool)));
    connect(abrBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(abrChanged(int)));
    connect(abrLowestBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(lowestChanged(int)));
    connect(abrHighestBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(highestChanged(int)));
}

//***************************************************************************
CompressionWidget::~CompressionWidget()
{
}

//***************************************************************************
void CompressionWidget::init(FileInfo &info)
{
    initInfo(lblCompressionNominalBitrate, abrBitrate,
             INF_BITRATE_NOMINAL, info);
    initInfo(0, abrHighestBitrate,
             INF_BITRATE_LOWER, info);
    initInfo(0, abrLowestBitrate,
             INF_BITRATE_UPPER, info);
    initInfo(lblCompressionBaseQuality, sbBaseQuality,
             INF_VBR_QUALITY, info);
    initInfo(0, slBaseQuality,
             INF_VBR_QUALITY, info);
}

//***************************************************************************
void CompressionWidget::describeWidget(QWidget *widget, const QString &name,
                                       const QString &description)
{
    if (!widget) return;
    QToolTip::add(widget, description);
    QWhatsThis::add(widget, "<b>"+name+"</b><br>"+description);
}

//***************************************************************************
void CompressionWidget::initInfo(QLabel *label, QWidget *widget,
                                 FileProperty property,
                                 FileInfo &info)
{
    Q_ASSERT(widget);
    if (label) label->setText(info.name(property) + ":");
    describeWidget(widget, info.name(property), info.description(property));
}

//***************************************************************************
void CompressionWidget::enableABR(bool enable, bool lowest, bool highest)
{
    rbABR->setEnabled(enable);
    if (!enable) rbABR->setChecked(false);
    
    if (!rbABR->isChecked()) {
	lblCompressionNominalBitrate->setEnabled(false);
	abrBitrate->setEnabled(false);
	abrHighestBitrate->setEnabled(false);
	abrLowestBitrate->setEnabled(false);
	chkHighestBitrate->setEnabled(false);
	chkLowestBitrate->setEnabled(false);
    }

    chkLowestBitrate->setChecked(lowest);
    chkHighestBitrate->setChecked(highest);
}

//***************************************************************************
void CompressionWidget::enableVBR(bool enable)
{
    rbVBR->setEnabled(enable);
    if (!enable) rbVBR->setChecked(false);

    if (!rbVBR->isChecked()) {
	lblCompressionBaseQuality->setEnabled(false);
	sbBaseQuality->setEnabled(false);
	slBaseQuality->setEnabled(false);
    }
}

//***************************************************************************
void CompressionWidget::selectABR(bool checked)
{
    abrHighestBitrate->setEnabled(checked && chkHighestBitrate->isChecked());
    abrLowestBitrate->setEnabled( checked && chkLowestBitrate->isChecked());
}

//***************************************************************************
void CompressionWidget::lowestToggled(bool on)
{
    if (on) {
	// if previous state was off: transition off->on
	// make sure that the lowest ABR is below the current ABR
	int abr = abrBitrate->value();
	if (abrLowestBitrate->value() > abr)
	    abrLowestBitrate->setValue(abr);
    }
    abrLowestBitrate->setEnabled(chkLowestBitrate->isEnabled() && on);
}


//***************************************************************************
void CompressionWidget::highestToggled(bool on)
{
    if (on) {
	// if previous state was off: transition off->on
	// make sure that the highest ABR is above the current ABR
	int abr = abrBitrate->value();
	if (abrHighestBitrate->value() < abr)
	    abrHighestBitrate->setValue(abr);
    }
    abrHighestBitrate->setEnabled(chkHighestBitrate->isEnabled() && on);
}


//***************************************************************************
void CompressionWidget::abrChanged(int value)
{
    if (value < abrLowestBitrate->value())
	abrLowestBitrate->setValue(value);
    if (value > abrHighestBitrate->value())
	abrHighestBitrate->setValue(value);
}

//***************************************************************************
void CompressionWidget::lowestChanged(int value)
{
    if (value > abrBitrate->value())
	abrBitrate->setValue(value);
    if (value > abrHighestBitrate->value())
	abrHighestBitrate->setValue(value);
}

//***************************************************************************
void CompressionWidget::highestChanged(int value)
{
    if (value < abrLowestBitrate->value())
	abrLowestBitrate->setValue(value);
    if (value < abrBitrate->value())
	abrBitrate->setValue(value);
}

//***************************************************************************
void CompressionWidget::setBitrates(int nominal, int lower, int upper)
{
    abrLowestBitrate->setValue(lower);
    abrHighestBitrate->setValue(upper);
    abrBitrate->setValue(nominal);
}

//***************************************************************************
void CompressionWidget::setQuality(int quality)
{
    sbBaseQuality->setValue(quality);
}

//***************************************************************************
CompressionWidget::Mode CompressionWidget::mode()
{
    return (rbABR->isChecked()) ? ABR_MODE : VBR_MODE;
}

//***************************************************************************
void CompressionWidget::setMode(CompressionWidget::Mode mode)
{
    switch (mode) {
	case ABR_MODE:
	    rbVBR->setChecked(false);
	    rbABR->setChecked(true);
	    break;
	case VBR_MODE:
	    rbABR->setChecked(false);
	    rbVBR->setChecked(true);
	    break;
    }
}

//***************************************************************************
bool CompressionWidget::lowestEnabled()
{
    return chkLowestBitrate->isChecked();
}

//***************************************************************************
bool CompressionWidget::highestEnabled()
{
    return chkHighestBitrate->isChecked();
}

//***************************************************************************
void CompressionWidget::getABRrates(int &nominal, int &lowest, int &highest)
{
    nominal = abrBitrate->value();
    lowest  = abrLowestBitrate->value();
    highest = abrHighestBitrate->value();
}

//***************************************************************************
int CompressionWidget::baseQuality()
{
    return sbBaseQuality->value();
}

//***************************************************************************
//***************************************************************************
