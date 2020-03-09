/*
 * CStrategyManager.h
 *
 *  Created on: 2017年10月19日
 *      Author: hongxu
 */

#ifndef SRC_TA_CSTRATEGYMANAGER_H_
#define SRC_TA_CSTRATEGYMANAGER_H_

#include "CCommanderBase.h"
#include "strategy_shared_comm.h"


class CStrategyManager: public CCommanderBase
{
public:
	CStrategyManager(CWaiter *p_owner);
	virtual ~CStrategyManager();

	int run(string &pkg);

	int status();

	static bool commInit();

	static bool registerStg(string name);

	static bool exitStg(string name);

	static bool checkStgExist(string name);

	static SHARED_STG_TNODE* getStg(string name);

private:

	static bool					s_is_init;
	static SHARED_STG_TABLE		s_table_;
};

#endif /* SRC_TA_CSTRATEGYMANAGER_H_ */
