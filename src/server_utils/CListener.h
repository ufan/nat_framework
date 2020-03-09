/*
 * CListener.h
 *
 *  Created on: 2018年4月16日
 *      Author: hongxu
 */

#ifndef SRC_MAPPORT_CLISTENER_H_
#define SRC_MAPPORT_CLISTENER_H_

#include <string>
#include "CSocket.h"
using namespace std;

class CListener: public CSocket
{
public:
	CListener();
	virtual ~CListener();

	bool open(string ip, uint16_t port);

	virtual void onAccept(int fd) = 0;

	void run() {start(EV_READ);}

	virtual void read_cb(EV_P_ ev_io *w, int events);

	virtual void write_cb(EV_P_ ev_io *w, int events) {}

	virtual void err_cb(EV_P_ ev_io *w, int events);
};

#endif /* SRC_MAPPORT_CLISTENER_H_ */
