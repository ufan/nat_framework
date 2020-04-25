/*
 * CMDEngineEESOnload.h
 *
 *  Created on: Aug 1, 2018
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDENGINEEESONLOAD_H_
#define SRC_MD_CMDENGINEEESONLOAD_H_

#include "IMDEngine.h"

class CMDEngineEESOnload: public IMDEngine
{
	CMDEngineEESOnload();
public:
	virtual ~CMDEngineEESOnload();

	void processMd(const void *);

	static CMDEngineEESOnload	instance_;

 protected:
	virtual bool init();

	virtual bool start();

	virtual void join();

	virtual void release();

	virtual void subscribe(const vector<string> &instr);

	virtual void unsubscribe(const vector<string> &instr);
};

#endif /* SRC_MD_CMDENGINEEESONLOAD_H_ */
