/*************************************************************************
       AsciiEncoder.cpp  -  encoder for ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include <qmemarray.h>
#include <qvaluevector.h>
#include <qvariant.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kglobal.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "AsciiCodecPlugin.h"
#include "AsciiEncoder.h"

/***************************************************************************/
AsciiEncoder::AsciiEncoder()
    :Encoder(), m_dst()
{
    m_dst.setEncoding(QTextOStream::UnicodeUTF8);
    LOAD_MIME_TYPES;
}

/***************************************************************************/
AsciiEncoder::~AsciiEncoder()
{
}

/***************************************************************************/
Encoder *AsciiEncoder::instance()
{
    return new AsciiEncoder();
}

/***************************************************************************/
QValueList<FileProperty> AsciiEncoder::supportedProperties()
{
    // default is to support all known properties
    FileInfo info;
    return info.allKnownProperties();
}

/***************************************************************************/
bool AsciiEncoder::encode(QWidget *widget, MultiTrackReader &src,
                          QIODevice &dst, FileInfo &info)
{
    bool result = true;

    qDebug("AsciiEncoder::encode()");

    // get info: tracks, sample rate
    unsigned int tracks = info.tracks();
    unsigned int bits   = info.bits();
    unsigned int length = info.length();

    do {
	// open the output device
	if (!dst.open(IO_ReadWrite | IO_Truncate)) {
	    KMessageBox::error(widget,
		i18n("Unable to open the file for saving!"));
	    result = false;
	    break;
	}

	// output device successfully opened
	m_dst.setDevice(&dst);

	// write out the default properties:
	// sample rate, bits, tracks, length
	m_dst << META_PREFIX << "'rate'="   << info.rate() << endl;
	m_dst << META_PREFIX << "'tracks'=" << tracks << endl;
	m_dst << META_PREFIX << "'bits'="   << bits   << endl;
	m_dst << META_PREFIX << "'length'=" << length << endl;

	// write out all other, non-standard properties that we have
	QMap<FileProperty, QVariant> properties = info.properties();
	QMap<FileProperty, QVariant>::Iterator it;
	QValueList<FileProperty> supported = supportedProperties();
	for (it=properties.begin(); it != properties.end(); ++it) {
	    FileProperty p = it.key();
	    QVariant     v = it.data();

	    if (!supported.contains(p))
		continue;
	    if (!info.canLoadSave(p))
		continue;

	    // write the property
	    m_dst << META_PREFIX << "'" << info.name(p) << "'='"
	          << v.toString().utf8() << "'" << endl;
	}

	unsigned int rest = length;
	unsigned int pos  = 0;
	while (rest-- && !src.isCancelled()) {
	    // write out one track per line
	    for (unsigned int track=0; track < tracks; track++) {
		SampleReader *reader = src[track];
		Q_ASSERT(reader);
		if (!reader) break;

		// read one single sample
		sample_t sample;
		(*reader) >> sample;

		// print out the sample value
		m_dst.width(9);
		m_dst << sample;

		// comma as separator between the samples
		if (track != tracks-1)
		    m_dst << ", ";
	    }

	    // as comment: current position [samples]
	    m_dst << " # ";
	    m_dst.width(12);
	    m_dst << pos;
	    pos++;

	    // end of line
	    m_dst << endl;
	}

    } while (false);

    // end of file
    m_dst << "# EOF " << endl << endl;

    m_dst.setDevice(0);
    dst.close();

    return result;
}

/***************************************************************************/
/***************************************************************************/