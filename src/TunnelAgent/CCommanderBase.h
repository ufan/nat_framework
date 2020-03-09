/*
 * CCommanderBase.h
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#ifndef SRC_TA_CCOMMANDERBASE_H_
#define SRC_TA_CCOMMANDERBASE_H_

#include <string>
#include "protocol.h"
using namespace std;

class CWaiter;

class CCommanderBase
{
public:
	CCommanderBase(CWaiter *p_owner);
	virtual ~CCommanderBase();

	virtual int run(string &pkg) { return STATE_ABORT; }

	void sendToClient(string &content);

protected:
	CWaiter		*p_owner_;
};


#endif /* SRC_TA_CCOMMANDERBASE_H_ */
