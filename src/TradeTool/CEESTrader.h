
#ifndef _CEESTRADER_H_
#define _CEESTRADER_H_

#include <stdint.h>
#include <vector>
#include <map>
#include <utility>
#include "CTraderBase.h"
#include "EesTraderApi.h"
using namespace std;

class CEESTrader : public EESTraderEvent, public CTraderBase
{
    enum emStatus
    {
        INIT,
        CONNECT,
        LOGOUT,
        LOGIN,
        STATUS_ERR,
        ON_QUERY,
        WAIT_QUERY,
    };

public:
    CEESTrader();
    virtual ~CEESTrader();

	virtual bool init();

	virtual int sendOrder(string instrument, double price, int volume, int direction, int offset);

	virtual int deleteOrder(string ref);

	virtual int qryAccount();

	virtual int qryPosition(const char *inst_idstr=NULL);

	virtual int qryOrder();

	virtual int qryTrade();

	virtual int qryTradeMargin();

	virtual int qryTradeFee();

	virtual void join() {}

    bool loadEESLib();

    bool connect();

    void login();

    void UnloadLib();

    void Close();

    bool getAccount();

    bool getSymbolList();

    virtual void OnConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnUserLogon(EES_LogonResponse* pLogon);
    virtual void OnQueryUserAccount(EES_AccountInfo* pAccoutnInfo, bool bFinish);
    virtual void OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId);
    virtual void OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish);
    virtual void OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish);
    virtual void OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish);
    virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);
    virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);
    virtual void OnOrderReject(EES_OrderRejectField* pReject);
    virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);
    virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);
    virtual void OnOrderExecution(EES_OrderExecutionField* pExec);
    virtual void OnOrderCxled(EES_OrderCxled* pCxled);
    virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);
    virtual void OnQueryAccountTradeMargin(const char* pAccount, EES_AccountMargin* pSymbolMargin, bool bFinish);
    virtual void OnQueryAccountTradeFee(const char* pAccount, EES_AccountFee* pSymbolFee, bool bFinish);

protected:
    int                         status_;
    EESTraderApi*				trade_api_;				///< EES交易API接口
	void                        *handle_;				///< EES交易API句柄
	funcDestroyEESTraderApi		distory_fun_;			///< EES交易API动态库销毁函数
    int                         req_id_;
    EES_UserID                  user_id_;

    vector<EES_AccountInfo>     accounts_;              // 用户下包含的账户
    int                         id_;

    map<pair<EES_UserID, EES_ClientToken>, string>    id_inst_map_;
    map<string, EES_ExchangeID>     symbol_info_;
};


#endif
