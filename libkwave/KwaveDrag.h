/***************************************************************************
            KwaveDrag.h  -  Drag&Drop container for Kwave's audio data
			     -------------------
    begin                : Jan 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _KWAVE_DRAG_H_
#define _KWAVE_DRAG_H_

#include "config.h"
#include <qcstring.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qobject.h>

class QImage;
class QMimeSource;
class QWidget;

class MultiTrackReader;
class Signal;

/**
 * Simple class for drag & drop of wav data.
 * @todo support for several codecs
 * @todo the current storage mechanism is straight-forward and stupid, it
 *       should be extended to use virtual memory
 */
class Q_EXPORT KwaveDrag: public QDragObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @see QDragObject
     */
    KwaveDrag(QWidget *dragSource = 0, const char *name = 0);

    /** Destructor */
    virtual ~KwaveDrag();

    /**
     * Returns one entry in the list of supported formats.
     * @param i index of the format [0...n]
     * @return name of the format or null
     */
    virtual const char *format(int i) const;

    /**
     * Returns the encoded data as a byte array.
     * @param format the mime type of the required data
     * @return byte array with wav encoded data
     */
    virtual QByteArray encodedData(const char *format) const;

    /**
     * Encodes wave data received from a MultiTrackReader into a byte
     * array that is compatible with the format of a wav file.
     * @param rate sample rate of the signal [samples/sec]
     * @param bits resolution [bits per sample]
     * @param src source of the samples
     * @return true if successful
     */
    bool encode(unsigned int rate, unsigned int bits,
                MultiTrackReader &src);

    /** Returns true if the mime type of the given source can be decoded */
    static bool canDecode(const QMimeSource* e);

    /**
     * Decodes the encoded byte data of the given mime source and
     * initializes a MultiTrackReader.
     * @param e mime source
     * @param sig signal that receives the mime data
     * @return true if successful, false if failed
     */
    static bool decode(const QMimeSource *e, Signal &sig);

private:

    /** simple array for storage of the wave data */
    QByteArray m_data;

};

#endif /* _KWAVE_DRAG_H_ */
