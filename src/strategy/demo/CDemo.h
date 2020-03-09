/*
 * CDemo.h
 *
 *  Created on: May 25, 2018
 *      Author: hongxu
 */

#ifndef SRC_STRATEGY_DEMO_CDEMO_H_
#define SRC_STRATEGY_DEMO_CDEMO_H_

#include "CStrategy.h"

class CDemo: public CStrategy
{
public:
	CDemo();
	virtual ~CDemo();

	virtual void on_tick(const UnitedMarketData*);
	virtual void on_time(long nano);
	virtual void on_rtn(const tRtnMsg*);
	virtual void on_switch_day(string day);
};

#endif /* SRC_STRATEGY_DEMO_CDEMO_H_ */
