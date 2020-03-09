/*
 * CTDEngineEES.h
 *
 *  Created on: Jul 20, 2018
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDENGINEEES_H_
#define SRC_TD_CTDENGINEEES_H_

#include <vector>
#include <string>
#include <stdint.h>
#include "ITDEngine.h"
#include "EesTraderApi.h"
#include "EesTraderDefine.h"


class CTDEngineEES : public ITDEngine, public EESTraderEvent
{
	struct AccountUnit
	{
	    EESTraderApi*			api 		= nullptr;
	    EES_EnterOrderField		order;
	    EES_CancelOrder			action_order;

	    bool					connected	= false;
	    bool					logged_in 	= false;
	};

public:
	CTDEngineEES();
	virtual ~CTDEngineEES();

	virtual string name() {return config_["/EESTD/name"_json_pointer];}

	virtual bool init(const json& j_conf);

	virtual void release();

	virtual int getAccountCnt() {return account_units_.size();}

	virtual bool updateOrderTrack();

    virtual bool is_connected() const;
    virtual bool is_logged_in() const;
    virtual void req_order_insert(const tIOInputOrderField* data);
    virtual void req_order_action(const tIOrderAction* data);
    virtual bool getBaseInfo() {return queryInstruments();}

    void connect();
    void login();
    bool queryInstruments();
	bool updateTradedAmount();

public:
    virtual void OnConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnUserLogon(EES_LogonResponse* pLogon);
    virtual void OnQueryUserAccount(EES_AccountInfo* pAccoutnInfo, bool bFinish);
    virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);
    virtual void OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish);
    virtual void OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish);

    virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);
    virtual void OnOrderReject(EES_OrderRejectField* pReject);
    virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);
    virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);
    virtual void OnOrderExecution(EES_OrderExecutionField* pExec);
    virtual void OnOrderCxled(EES_OrderCxled* pCxled);
    virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);

protected:
    vector<AccountUnit> 	account_units_;
    int						curAccountIdx_ = 0;
    bool					query_complete_flag_ = false;
    long 					max_ref_id_ = 0;
    json					config_;
};

#endif /* SRC_TD_CTDENGINEEES_H_ */
