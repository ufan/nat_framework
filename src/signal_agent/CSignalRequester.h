/*
 * CSignalRequester.h
 *
 *  Created on: Jun 26, 2018
 *      Author: hongxu
 */

#ifndef SRC_SIGNAL_AGENT_CSIGNALREQUESTER_H_
#define SRC_SIGNAL_AGENT_CSIGNALREQUESTER_H_

#include <string>
#include <vector>
#include <memory>
#include "CSignal.h"
#include "CService.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class CSignalRequester: public CService
{
public:
	CSignalRequester(long timeout, uint32_t buf_size=4*4096);
	virtual ~CSignalRequester();

	virtual int processHead(const char *p, uint32_t len);

	virtual int processPkg(const char *p, uint32_t len);

	virtual void close();

	virtual int onConnected();

	virtual void onConnectErr(int err);

	void timeout(EV_P_ ev_timer *w, int events);

	bool init(const json &j_conf);

	void sendRequest();

	void tryHeartBeat();

	bool testOK();

protected:
	string 			ip_;
	uint16_t		port_ = 0;
	vector<unique_ptr<CSignalWriter>>  writers_;
	bool			is_connect_ = false;
	long			lst_alive_time_ = 0;
	long			lst_heartbeat_time_ = 0;
	long			timeout_ = 5000000000L;
	vector<string>	signames_;
};

#endif /* SRC_SIGNAL_AGENT_CSIGNALREQUESTER_H_ */
