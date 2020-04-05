/*
 * CCtpTrader.h
 *
 *  Created on: 2017年12月19日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CCTPTRADER_H_
#define SRC_TRADER_CCTPTRADER_H_

#include <string>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "CTraderBase.h"
using namespace std;

class CCtpTrader : public CThostFtdcTraderSpi, public CTraderBase
{
public:
	CCtpTrader();
	virtual ~CCtpTrader();

	virtual bool init();

	virtual int sendOrder(string instrument, double price, int volume, int direction, int offset);

	virtual int deleteOrder(string ref);

	virtual int qryAccount();

	virtual int qryPosition(const char *inst_idstr=NULL);

	virtual int qryOrder();

	virtual int qryTrade();

	virtual void join();

	virtual void parseRetCode(int code, string prefix);

	virtual void reqSettlementInfoConfirm();

	virtual void reqUserLogin();

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *p, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *p, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

protected:
	CThostFtdcTraderApi *p_api_;
	int 				request_id_;
	int					order_id_;
	int					front_id_;
	int					session_id_;
	string				order_ref_;

	string				broker_id_;
	string 				user_id_;

	int					id_;
};

#endif /* SRC_TRADER_CCTPTRADER_H_ */
