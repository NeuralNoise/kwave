/***************************************************************************
			  AsyncSync.h  -  multithreading support for qt
			     -------------------
    begin                : Jun 1997
    copyright            : (C) 2000 by Brian Theodore,
                           Science Applications International Corp.
                           Simulation Technology Division
    email                : theodore@std.saic.com

  (copied to the kwave project by Thomas.Eschenbacher@gmx.de at Jun 2000)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ASYNC_SYNC_H_
#define _ASYNC_SYNC_H_

#include <qobject.h>

class QGList;
class QSocketNotifier;

class AsyncSync : public QObject
{
    Q_OBJECT

public :

    /**
     * Default Constructor
     */
    AsyncSync();

    /**
     * Default Destructor
     */
    virtual ~AsyncSync();

protected slots :

    /**
     * Slot called synchronously by the X server in response to the
     * asynchronous file descriptor it is watching having data ready.
     */
    virtual void SyncHandler(QGList &params);

public slots :

    /**
     * Slot to be called by asynchronous client.  This slot does not more
     * than write to pipe, which will trigger the X server to respond
     * to the file descriptor, and synchronously call the SyncHandler
     */
    virtual void AsyncHandler(QGList &params);

signals :

    /**
     * Signal emitted when the sync handler gets called
     */
    void Activated(QGList &params);

private :

    /**
     * IPC pipe for async/sync communication with X server
     */
    int fds[2];

    /**
     * Socket notifier to call slot when pipe has message to read
     */
    QSocketNotifier *sn;

};

#endif // _ASYNC_SYNC_H_
