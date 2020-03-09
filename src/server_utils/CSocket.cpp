/*
 * CSocket.cpp
 *
 *  Created on: 2018年4月13日
 *      Author: hongxu
 */

#include "CSocket.h"

CSocket::CSocket() : fd_(-1), is_watch_(false)
{
	io_.data = this;
}

CSocket::~CSocket()
{
	close();
}

void CSocket::close()
{
	if(fd_ >= 0)
	{
		stop();
		::close(fd_);
		fd_ = -1;
	}
}

void CSocket::read_write_cb(EV_P_ ev_io* w, int events)
{
	if(events & EV_ERROR)
	{
		err_cb(EV_A_ w, events);
	}

	if((events & EV_READ) && fd_ >= 0)
	{
		read_cb(EV_A_ w, events);
	}

	if((events & EV_WRITE) && fd_ >= 0)
	{
		write_cb(EV_A_ w, events);
	}
}

