/*
 * CTraderFactory.h
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CTRADERFACTORY_H_
#define SRC_TRADER_CTRADERFACTORY_H_

#include "CTraderBase.h"

class CTraderFactory
{
public:
	CTraderFactory();
	virtual ~CTraderFactory();

	static CTraderBase* getTrader(string desk);
};

#endif /* SRC_TRADER_CTRADERFACTORY_H_ */
