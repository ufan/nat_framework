/*
 * CSignalAgent.h
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 */

#ifndef SRC_SIGNAL_AGENT_CSIGNALAGENT_H_
#define SRC_SIGNAL_AGENT_CSIGNALAGENT_H_

#include <vector>
#include <memory>
#include "CListener.h"
#include "CSignalServer.h"
#include "CSignalRequester.h"
#include "Logger.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class CSignalAgent : public CListener
{
public:
	CSignalAgent();
	virtual ~CSignalAgent();

	virtual void onAccept(int fd);

	void process();

	bool init(const json &j_conf);

protected:
	vector<unique_ptr<CSignalServer>>		servers_;
	vector<unique_ptr<CSignalRequester>>	requesters_;
	long	server_scan_period_ 			= 0;
	long	server_unload_scan_period_		= 0;
	long	timeout_						= 0;
	long	server_lst_scan_				= 0;
	long	server_lst_scan_unload_			= 0;
	uint32_t buf_size_						= 4 * 4096;

	ev_idle idle_;
};

#endif /* SRC_SIGNAL_AGENT_CSIGNALAGENT_H_ */
