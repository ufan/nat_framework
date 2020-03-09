/*
 * CService.h
 *
 *  Created on: 2018年4月13日
 *      Author: sky
 */

#ifndef MAPPORT_CSERVICE_H_
#define MAPPORT_CSERVICE_H_

#include <string>
#include "CSocket.h"
using namespace std;

class CService: public CSocket
{
protected:
	enum
	{
		READ_HEAD,
		READ_BODY,
	};

public:
	CService(uint32_t rbuf_size=4096, uint32_t wbuf_size=4096);
	virtual ~CService();

	virtual int processHead(const char *p, uint32_t len);

	virtual int processPkg(const char *p, uint32_t len);

	virtual int onConnected() {return 0;}

	virtual void onConnectErr(int err);

	// will auto close later
	virtual void onErr(int err);

	virtual void close();

	virtual int sendData(const char *p, uint32_t len);

	virtual void read_cb(EV_P_ ev_io *w, int events);

	virtual void write_cb(EV_P_ ev_io *w, int events);

	virtual void err_cb(EV_P_ ev_io *w, int events) { onErr(errno); close(); }

	virtual void connect_cb(EV_P_ ev_io *w, int events);

	virtual void connect_timeout_cb(EV_P_ ev_timer *w, int events);

	// size should not larger than read buffer size
	bool setHeadSize(uint32_t size);

	// size should not larger than read buffer size
	bool setPkgSize(uint32_t size);

	bool connect(string ip, uint16_t port, ev_tstamp timeout);

protected:
	uint32_t			head_size_;
	uint32_t			pkg_size_;
	int					state_;
	CBuffer				read_buf_;
	CBuffer				write_buf_;

	ev_timer    		timer_;
};

#endif /* MAPPORT_CSERVICE_H_ */

