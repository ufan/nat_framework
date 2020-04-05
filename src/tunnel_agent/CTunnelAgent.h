/*
 * CTunnelAgent.h
 *
 *  Created on: 2017年9月26日
 *      Author: hongxu
 */

#ifndef SRC_TA_CTUNNELAGENT_H_
#define SRC_TA_CTUNNELAGENT_H_

#include <string>
#include <map>
#include <memory>
#include "CWaiter.h"
#include "CConfig.h"
#include "Logger.h"
#include "ev.h"
#include "compiler.h"
using namespace std;


class CTunnelAgent
{
	CTunnelAgent();
public:
	virtual ~CTunnelAgent();

	bool init(string cfg_path);

	void run();

	void releaseWaiter(int fd);

	static void accept_cb(EV_P_ ev_io *w, int events);

	static void terminate_cb(EV_P_ ev_signal *w, int events);

	static CConfig* getConfig();

	static CTunnelAgent* instance();

private:
	int listen_fd_;
	map<int, unique_ptr<tWaiterWatcher> > waiters_;
};

#endif /* SRC_TA_CTUNNELAGENT_H_ */
