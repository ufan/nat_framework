/*
 * CDemoStrategy.h
 *
 *  Created on: 2018年5月10日
 *      Author: hongxu
 */

#ifndef SRC_STRATEGY_DEMO_CDEMOSTRATEGY_H_
#define SRC_STRATEGY_DEMO_CDEMOSTRATEGY_H_

#include "CStrategyProcess.h"

class CDemoStrategy: public CStrategyBase
{
public:
	CDemoStrategy();
	virtual ~CDemoStrategy();

	virtual void on_tick(const UnitedMarketData*);
	virtual void on_time(long nano);
	virtual void on_rtn(const tRtnMsg*);
};

#endif /* SRC_STRATEGY_DEMO_CDEMOSTRATEGY_H_ */
