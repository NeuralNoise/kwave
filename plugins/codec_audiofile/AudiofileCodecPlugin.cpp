/*************************************************************************
    AudiofileCodecPlugin.cpp  -  import/export through libaudiofile
                             -------------------
    begin                : Tue May 28 2002
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

#include "config.h"

#include "kwave/CodecManager.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"

KWAVE_PLUGIN(AudiofileCodecPlugin,"codec_audiofile","Thomas Eschenbacher");

/***************************************************************************/
AudiofileCodecPlugin::AudiofileCodecPlugin(PluginContext &c)
    :KwavePlugin(c), m_decoder(0)
{
    i18n("codec_audiofile");
}

/***************************************************************************/
AudiofileCodecPlugin::~AudiofileCodecPlugin()
{
}

/***************************************************************************/
void AudiofileCodecPlugin::load(QStringList &/* params */)
{
    if (!m_decoder) m_decoder = new AudiofileDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
/***************************************************************************/
