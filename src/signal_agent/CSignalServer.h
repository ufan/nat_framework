/*
 * CSignalServer.h
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 */

#ifndef SRC_SIGNAL_AGENT_CSIGNALSERVER_H_
#define SRC_SIGNAL_AGENT_CSIGNALSERVER_H_

#include <memory>
#include <vector>
#include <set>
#include "CService.h"
#include "CSignal.h"
using namespace std;


class CSignalServer: public CService
{
public:
	CSignalServer(int fd, long timeout, uint32_t buf_size=4*4096);
	virtual ~CSignalServer() {}

	virtual int processHead(const char *p, uint32_t len);

	virtual int processPkg(const char *p, uint32_t len);

	void addReader(string sig_name);

	void tryRead(bool scan_unload=false);

	bool isOK();

protected:
	vector<unique_ptr<CSignalReader>>	readers_;
	long		lst_alive_time_ = 0;
	long		timeout_ = 5000000000L;
	set<string>	read_signal_set_;
};


#endif /* SRC_SIGNAL_AGENT_CSIGNALSERVER_H_ */
