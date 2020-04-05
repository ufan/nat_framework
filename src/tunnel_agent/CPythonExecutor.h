/*
 * CPythonExecutor.h
 *
 *  Created on: 2017年9月28日
 *      Author: hongxu
 */

#ifndef SRC_TA_CPYTHONEXECUTOR_H_
#define SRC_TA_CPYTHONEXECUTOR_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "CDirReciever.h"
#include "CFileReciever.h"
#include "ev.h"
using namespace std;

class CPythonExecutor : public CFileReciever
{
	enum emPyExecState
	{
		STATE_RECV_SCRIPT,
		STATE_RECV_CONF,
		STATE_RECV_WORKDIR,
		STATE_RECV_ARGS,
		STATE_RUN,
	};

	struct tChildWatcher
	{
		ev_child wathcer;
		string 	 name;
	};

	struct tDelWatcher
	{
		ev_timer timer;
		string   file;
		string   dirpath;
	};

public:
	CPythonExecutor(CWaiter *p_owner);
	virtual ~CPythonExecutor();

	int run(string &pkg);

	int execute();

	static void child_cb(EV_P_ ev_child* w, int revents);

private:
	string 						conf_;
	string 						args_;
	unique_ptr<CFileReciever>	p_conf_reciever_;
	unique_ptr<CDirReciever>	p_dir_reciever_;
	int							state_;
};

#endif /* SRC_TA_CPYTHONEXECUTOR_H_ */
