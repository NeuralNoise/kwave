/***************************************************************************
         CodecManager.h  -  manager for Kwave's coders and decoders
			     -------------------
    begin                : Mar 10 2002
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

#ifndef _CODEC_MANAGER_H_
#define _CODEC_MANAGER_H_

#include "config.h"
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>

class Decoder;
class Encoder;
class KMimeType;
class QMimeSource;

class CodecManager: public QObject
{
    Q_OBJECT
public:
    /** Constructor */
    CodecManager();

    /** Destructor */
    virtual ~CodecManager();

    /**
     * Registers a new encoder.
     * @param encoder a KwaveEncoder that will be used as template for
     *                creating new encoder instances (used as factory)
     */
    static void registerEncoder(const Encoder &encoder);

    /**
     * Registers a new decoder.
     * @param decoder a KwaveDecoder that will be used as template for
     *                creating new decoder instances (used as factory)
     */
    static void registerDecoder(const Decoder &decoder);

    /**
     * Returns true if a decoder for the given mime type is known.
     * @param mimetype mime type describing the audio format
     * @return true if format is supported, false if not
     */
    static bool canDecode(const KMimeType &mimetype);

    /**
     * Returns true if a decoder for the given mime type is known.
     * @param mimetype_name name of the mime type
     * @return true if format is supported, false if not
     */
    static bool canDecode(const QString &mimetype_name);

    /**
     * Tries to find a decoder that matches to a given mime type.
     * @param mimetype mime type of the source
     * @return a new decoder for the mime type or null if none found.
     */
    static Decoder *decoder(const KMimeType &mimetype);

    /**
     * Same as above, but takes the mime type as string.
     * @param mimetype_name name of the mime type
     * @return a new decoder for the mime type or null if none found.
     */
    static Decoder *decoder(const QString &mimetype_name);

    /**
     * Same as above, but takes the mime info from a QMimeSource.
     * @param mime_source source with a mime type (null pointer is allowed)
     * @return a new decoder for the mime type or null if none found.
     */
    static Decoder *decoder(const QMimeSource *mime_source);

    /**
     * Tries to find an encoder that matches to a given mime type.
     * @param mimetype_name name of the mime type of the destination
     * @return a new encoder for the mime type or null if none found.
     */
    static Encoder *encoder(const QString &mimetype_name);

    /**
     * Returns a string with a list of all file types that can be used
     * for saving. The string is localized and can be used as a filter
     * for a KFileDialog. The entries are unique (by file extension) but
     * not sorted alphabetically.
     */
    static QString encodingFilter();

    /**
     * Returns a string with a list of all file types that can be used
     * for loading. The string is localized and can be used as a filter
     * for a KFileDialog. The entries are unique (by file extension) but
     * not sorted alphabetically.
     */
    static QString decodingFilter();


    /**
     * Tries to find the name of a mime type of a decoder by a URL.
     * If not found, it returns the default mime type, never an empty string.
     * @param url a KURL, only the filename's extension will be inspected
     * @return name of the mime type or the default mime type
     */
    static QString whatContains(const KURL &url);

private:
    /** list of all encoders */
    static QPtrList<Encoder> m_encoders;

    /** list of decoders */
    static QPtrList<Decoder> m_decoders;
};

#endif /* _CODEC_MANAGER_H_ */