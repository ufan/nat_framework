/*
 * CWaiter.h
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#ifndef SRC_TA_CWAITER_H_
#define SRC_TA_CWAITER_H_

#include <memory>
#include "CCommanderBase.h"
#include "ev.h"
#include "CCrypt.h"
using namespace std;

class CWaiter
{
	enum
	{
		INIT,
		SSL_AUTH,
		GET_DES_KEY,
		DO_COMMAND,
		FINISH,
	};

public:
	CWaiter(int fd);
	virtual ~CWaiter();

	int process(EV_P_ ev_io *w, int events);

	bool write_cb();

	int read_cb();

	void resetListen(int events);

	void send(string &s);

	void desSendData(string &s);

	void sayFinish(uint8_t type=TYPE_FINISH_SUCC);

	void handleConnectionBroken();

	int getFd() {return fd_;}

	static void readWrite_cb(EV_P_ ev_io *w, int events);

public:
	int processPkg();

	int init();

	int processSSLAuth(string &pkg);

	int get3DESkey(string &pkg);

	int doCommand(string &pkg);

private:
	int			fd_;
	int 		state_;

	uint32_t	read_cur_;
	uint32_t	write_cur_;

	bool		is_write_open_;

	string		read_buf_;
	string 		write_buf_;

private:
	CCrypt		crypter_;
	unique_ptr<CCommanderBase>  p_commander_;
};


struct tWaiterWatcher
{
	ev_io 		watcher;
	CWaiter		waiter;

	tWaiterWatcher(int fd) : waiter(fd) {}

	static ev_io* getWatcher(CWaiter* p_waiter)
	{
		return (ev_io*)((const char*)p_waiter - sizeof(ev_io));
	}
};

#endif /* SRC_TA_CWAITER_H_ */

