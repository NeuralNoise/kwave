/*************************************************************************
 SampleDecoderLinear.cpp  -  decoder for all non-compressed linear formats
                             -------------------
    begin                : Sat Nov 01 2003
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

#include <stdio.h>
#include <sys/types.h>

#include "libkwave/CompressionType.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"

#include "SampleDecoderLinear.h"

//***************************************************************************
void decode_NULL(char *src, sample_t *dst, unsigned int count)
{
    while (count--) {
        printf("%02X ", (int)*src);
        *(dst++) = count % (1 << (SAMPLE_BITS-1));
    }
}

//***************************************************************************
// this little function is provided as inline code to avaid a compiler
// warning about negative shift value when included directly
static inline u_int32_t shl(const u_int32_t v, const int s)
{
    return (s > 0) ? (v << s) : (v >> s);
}

//***************************************************************************
/**
 * Template for decoding a buffer with linear samples. The tricky part is
 * done in the compiler which optimizes away all unused parts of current
 * variant and does nice loop optimizing!
 * @param src array with raw data
 * @param dst array that receives the samples in Kwave's format
 * @param count returns the number of samples to be decoded
 */
template<const unsigned int bits, const bool is_signed>
void decode_linear(char *src, sample_t *dst, unsigned int count)
{
    const int shift = (SAMPLE_BITS - bits);
    const u_int32_t sign = 1 << (SAMPLE_BITS-1);
    const u_int32_t negative = ~(sign - 1);

    while (count--) {
	// read from source buffer, assume little endian
	register u_int32_t s = 0;
	for (unsigned int byte=0; byte < ((bits+7)>>3); ++byte) {
	    s |= (unsigned char)(*(src++)) << (byte << 3);
	}

	// convert to signed
	if (!is_signed) s -= shl(1, bits-1)-1;

	// shift up to Kwave's bit count
	s = shl(s, shift);

	// sign correcture for negative values
	if (is_signed && (s & sign)) s |= negative;

	// write to destination buffer
	*(dst++) = static_cast<sample_t>(s);
    }
}

//***************************************************************************
SampleDecoderLinear::SampleDecoderLinear(int sample_format,
                                         unsigned int bits_per_sample)
    :SampleDecoder(),
    m_bytes_per_sample((bits_per_sample + 7) >> 3),
    m_decoder(decode_NULL)
{
    switch (sample_format) {
	case AF_SAMPFMT_UNSIGNED:
	    switch (m_bytes_per_sample) {
	        case 1:
		    m_decoder = decode_linear<8, false>;
		    break;
		case 2:
		    m_decoder = decode_linear<16, false>;
		    break;
		case 3:
		    m_decoder = decode_linear<24, false>;
		    break;
		case 4:
		    m_decoder = decode_linear<32, false>;
		    break;
	    }
	    break;

	case AF_SAMPFMT_TWOSCOMP:
	    switch (m_bytes_per_sample) {
	        case 1:
		    m_decoder = decode_linear<8, true>;
		    break;
		case 2:
		    m_decoder = decode_linear<16, true>;
		    break;
		case 3:
		    m_decoder = decode_linear<24, true>;
		    break;
		case 4:
		    m_decoder = decode_linear<32, true>;
		    break;
	    }
	    break;
    }
}

//***************************************************************************
SampleDecoderLinear::~SampleDecoderLinear()
{
}

//***************************************************************************
void SampleDecoderLinear::decode(QByteArray &raw_data,
                                 QMemArray<sample_t> &decoded)
{
    Q_ASSERT(m_decoder);
    if (!m_decoder) return;

    unsigned int samples = raw_data.size() / m_bytes_per_sample;
    char *src = raw_data.data();
    sample_t *dst = decoded.data();

    m_decoder(src, dst, samples);
}

//***************************************************************************
unsigned int SampleDecoderLinear::rawBytesPerSample()
{
    return m_bytes_per_sample;
}

//***************************************************************************
//***************************************************************************
