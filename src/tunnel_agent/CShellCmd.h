/*
 * CShellCmd.h
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#ifndef SRC_TA_CSHELLCMD_H_
#define SRC_TA_CSHELLCMD_H_

#include "CCommanderBase.h"
#include "ev.h"

class CShellCmd: public CCommanderBase
{
	enum
	{
		START,
		RUNNING,
		STOP,
	};

public:
	CShellCmd(CWaiter *p_owner);
	virtual ~CShellCmd();

	virtual int run(string &pkg);

	void onChildExit();

	void onRead();

	static void child_cb (EV_P_ ev_child *w, int revents);

	static void read_cb(EV_P_ ev_io *w, int revents);

private:
	string		read_buf_;
	int			run_state_;
	ev_child 	child_watcher_;
	ev_io		read_watcher_;
};

#endif /* SRC_TA_CSHELLCMD_H_ */
