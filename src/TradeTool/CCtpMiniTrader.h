/*
 * CCtpMiniTrader.h
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CCTPMINITRADER_H_
#define SRC_TRADER_CCTPMINITRADER_H_

#include "CCtpTrader.h"
#include "CGlobalParameter.h"

class CCtpMiniTrader : public CCtpTrader
{
public:
	CCtpMiniTrader();
	virtual ~CCtpMiniTrader();

	virtual bool init();

	virtual void reqUserLogin();

	virtual void reqSettlementInfoConfirm() { CGlobalParameter::terminal_lock_ = false; }
};

#endif /* SRC_TRADER_CCTPMINITRADER_H_ */
