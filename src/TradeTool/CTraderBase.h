/*
 * CTraderBase.h
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CTRADERBASE_H_
#define SRC_TRADER_CTRADERBASE_H_

#include <string>
using namespace std;

class CTraderBase
{
public:
	CTraderBase();
	virtual ~CTraderBase();

	virtual bool init() = 0;

	virtual int sendOrder(string instrument, double price, int volume, int direction, int offset) = 0;

	virtual int deleteOrder(string ref) = 0;

	virtual int qryAccount() = 0;

	virtual int qryPosition(const char *inst_idstr=NULL) = 0;

	virtual int qryOrder() = 0;

	virtual int qryTrade() = 0;

	virtual int qryTradeMargin();

	virtual int qryTradeFee();

	virtual void join() = 0;
};

#endif /* SRC_TRADER_CTRADERBASE_H_ */
