/***************************************************************************
            MetaData.h  -  base class for associated meta data
                             -------------------
    begin                : Sat Jan 23 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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
#ifndef _META_DATA_H_
#define _META_DATA_H_

#include "config.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QVariant>

#include <kdemacros.h>

namespace Kwave {

    class KDE_EXPORT MetaData
    {
    public:
	/** standard property: list of zero based track indices */
	static const QString STDPROP_TRACKS;

	/** standard property: start sample index (inclusive) */
	static const QString STDPROP_START;

	/** standard property: end sample index (inclusive) */
	static const QString STDPROP_END;

	/** standard property: position [zero based sample index] */
	static const QString STDPROP_POS;

	/** standard property: description (string) */
	static const QString STDPROP_DESCRIPTION;

	typedef enum
	{
	    /** no scope */
	    None     = 0,

	    /** whole signal */
	    Signal   = (1 << 0),

	    /**
	     * bound to a (list of) tracks, requires the property
	     * "STDPROP_TRACKS" with a QVariant::List of track indices
	     * encoded as QVariant::UInt
	     */
	    Track    = (1 << 1),

	    /**
	     * bound to a range of samples, requires the properties
	     * "STDPROP_START" and "STDPROP_END" with data type
	     * QVariant::ULongLong
	     */
	    Range    = (1 << 2),

	    /**
	     * bound to a single sample, requires the property
	     * "STDPROP_POS" with data type "QVariant::ULongLong"
	     */
	    Position = (1 << 3),

	    /** can be used for selecting all the scopes above */
	    All      = ~0
	} Scope;

	/** List of metadata properties */
	typedef QMap<QString, QVariant> PropertyList;

	/** default constructor */
	MetaData();

	/** constructor */
	MetaData(Scope scope);

	/** destructor */
	virtual ~MetaData();

	/** returns the scope of the meta data */
	Scope scope() const;

	/**
	 * Sets the the scope of the meta data
	 * @param scope the new scope
	 */
	void setScope(Scope scope);

	/**
	 * Sets a property to a new value. If the property already exists
	 * it will be created and if it did not exist, a new one will be
	 * created.
	 * @param p name of the property
	 * @param value a QVariant with the property's data
	 */
	void setProperty(const QString &p, const QVariant &value);

	/**
	 * Checks whether this metadata object contains a given property.
	 * @param p name of the property
	 * @return true if the property exists, false otherwise
	 */
	bool hasProperty(const QString &p) const;

	/**
	 * Returns a QVariant with the copy of the value of a property
	 * or an empty QVariant if the property does not exist.
	 * @param p name of the property
	 * @return value of the property or an empty QVariant
	 */
	QVariant property(const QString &p) const;

	/** Same as above, for using through the [] operator */
	QVariant operator [] (const QString p) const
	{
	    return property(p);
	}

	/**
	 * Returns a mutable reference to an existing property (or the
	 * reference to an empty dummy if it did not exist).
	 * @param p name of the property
	 * @return reference to the value of the property
	 */
	QVariant &property(const QString &p);

	/** Same as above, for using through the [] operator */
	QVariant &operator [] (const QString p)
	{
	    return property(p);
	}

    private:

	/** internal container class with meta data */
	class MetaDataPriv: public QSharedData
	{
	public:

	    /** constructor */
	    MetaDataPriv();

	    /** copy constructor */
	    MetaDataPriv(const MetaDataPriv &other);

	    /** destructor */
	    virtual ~MetaDataPriv();

	    /** scope of the meta data */
	    Scope m_scope;

	    /** list of properties, user defined */
	    PropertyList m_properties;
	};

	/** pointer to the shared meta data */
	QSharedDataPointer<MetaDataPriv> m_data;
    };
}

#endif /* _META_DATA_H_ */
