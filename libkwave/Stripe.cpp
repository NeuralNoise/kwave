/***************************************************************************
             Stripe.cpp  -  continuous block of samples
			     -------------------
    begin                : Feb 10 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "mt/MutexGuard.h"

#include "libkwave/Stripe.h"

//***************************************************************************
Stripe::Stripe()
    :m_lock(), m_samples(), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start, unsigned int length)
    :m_lock(), m_start(start), m_samples(), m_lock_samples()
{
    m_samples.resize(length);
    ASSERT(m_samples.size() == length);
}

//***************************************************************************
Stripe::Stripe(unsigned int start, const QArray<sample_t> &samples)
    :m_lock(), m_start(start), m_samples(samples), m_lock_samples()
{
}

//***************************************************************************
Mutex &Stripe::mutex()
{
    return m_lock;
}


//***************************************************************************
unsigned int Stripe::start()
{
    MutexGuard lock(m_lock_samples);
    return m_start;
}

//***************************************************************************
unsigned int Stripe::length()
{
    MutexGuard lock(m_lock_samples);
    return m_samples.size();
}

//***************************************************************************
unsigned int Stripe::resize(unsigned int length)
{
    MutexGuard lock(m_lock_samples);

    unsigned int old_size = m_samples.size();
    if (old_size == length) return old_size; // nothing to do

    debug("resizing stripe from %d to %d samples", old_size, length);
    m_samples.resize(length);
    ASSERT(length = m_samples.size());
    if (length < m_samples.size()) {
	warning("resize failed, out of memory ?");
    }

    length = m_samples.size();

    // fill new samples with zero
    while (old_size < length) {
	m_samples[old_size++] = 0;
    }

    return length;
}

//***************************************************************************
void Stripe::operator << (const QArray<sample_t> &samples)
{
    if (!samples.count()) return; // nothing to do

    MutexGuard lock(m_lock_samples);
    debug("Stripe::operator << : adding %d samples", samples.count());

    unsigned int newlength = m_samples.size() + samples.count();
    m_samples.resize(newlength);
    ASSERT(newlength = m_samples.size());

}

//***************************************************************************
//***************************************************************************
