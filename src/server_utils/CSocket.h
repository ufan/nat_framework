/*
 * CSocket.h
 *
 *  Created on: 2018年4月13日
 *      Author: hongxu
 */

#ifndef SRC_MAPPORT_CSOCKET_H_
#define SRC_MAPPORT_CSOCKET_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "compiler.h"
#include "ev.h"
#include "Logger.h"
#include "CBuffer.h"


class CSocket
{
public:
	CSocket();
	virtual ~CSocket();

	virtual void close();

	virtual void read_write_cb(EV_P_ ev_io *w, int events);

	virtual void read_cb(EV_P_ ev_io *w, int events) = 0;

	virtual void write_cb(EV_P_ ev_io *w, int events) = 0;

	virtual void err_cb(EV_P_ ev_io *w, int events) {}

	void setFd(int fd) {fd_ = fd;}

	int	getFd() {return fd_;}

protected:
	int				fd_;

// ev_io relevant
public:
	void stop()
	{
		if(is_watch_)
		{
			ev_io_stop (EV_DEFAULT, &io_);
			is_watch_ = false;
		}
	}

	void start(int events)
	{
		stop();

		is_watch_ = true;
		ev_io_init (&io_, ev_read_write_cb, fd_, events);
		ev_io_start (EV_DEFAULT, &io_);
	}

	void stop(int events)
	{
		if(is_watch_)
		{
			int e = io_.events & ~ events;
			if(e)
			{
				if(e != io_.events)
				{
					start(e);
				}
			}
			else stop();
		}
	}

	void addEvent(int events)
	{
		int e = is_watch_ ? (io_.events | events) : events;
		if(e && (!is_watch_ || e != io_.events)) start(e);
	}

	static CSocket* getOwner(ev_io *w)
	{
		return static_cast<CSocket*>(w->data);
	}

	static void ev_read_write_cb(EV_P_ ev_io *w, int events)
	{
		getOwner(w)->read_write_cb(EV_A_ w, events);
	}

protected:
	ev_io 		io_;
	bool 		is_watch_;
};


#endif /* SRC_MAPPORT_CSOCKET_H_ */

