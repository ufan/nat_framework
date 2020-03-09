/*
 * CTDEngineXt.h
 *
 *  Created on: 2018年6月25日
 *      Author: Tommy
 *
 *  Note: Copy from CTDEngineXt.h
 */

#ifndef TD_CTDENGINEXT_H_
#define TD_CTDENGINEXT_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <unordered_map>
#include "ITDEngine.h"
#include "MarketType.h"
#include "XtDataType.h"
#include "XtDef.h"
#include "XtError.h"
#include "XtStructs.h"
#include "XtTraderApi.h"
using namespace std;
using namespace xti;

/**
 * account information unit extra for Xt is here.
 */
struct AccountUnitXt
{
    COrdinaryOrder input_order;

    XtTraderApi* api;

    // internal flags
    bool    initialized;
    bool    connected;
    bool    logged_in;
    int     login_rid;
	
    string account_id;
    string user_id;
    string passwd;
};

class CTDEngineXt: public ITDEngine, public XtTraderApiCallback
{
public:
	CTDEngineXt();
	virtual ~CTDEngineXt();

	virtual string name() {return name_;}

	virtual bool init(const json& j_conf);
	
	virtual void join();

	virtual void release();

	virtual int getAccountCnt() {return account_units_.size();}

	virtual bool updateOrderTrack();
	bool updateTradedAmount();

    void connect(long timeout_nsec, string trade_flow_path);
    void login(long timeout_nsec);
    void logout();
    void release_api();
    bool load_account(int idx, const json& j_account);
    bool queryInstruments(long timeout_nsec);
	EOperationType ConvJarvisOffsetToXtOffset(int dir, int off);
	int getMultipleByPrd(const char* prd);
	tOrderTrack* findOrderId(int order_id);

public:
    virtual bool is_connected() const;
    virtual bool is_logged_in() const;
    virtual void req_order_insert(const tIOInputOrderField* data);
    virtual void req_order_action(const tIOrderAction* data);
    virtual bool getBaseInfo();
	
public:
        // 相应请求的回调函数
        // @param   nRequestId 和请求函数中 nRequestId 想对应
        // @param   error 请求函数的返回是否成功，如果失败有错误信息
        // @remark  函数名组成：以 on 为前缀，加上相应请求函数的函数名称（请求函数首字母大写）

        // 连接服务器的回调函数
        // @param success   服务器连接是否成
        // @param errorMsg  如果服务器连接失败，存储错误信息
        virtual void onConnected(bool success, const char* errorMsg);
        // 客户端用户登录的回调函数
        virtual void onUserLogin(const char* userName, const char* password, int nRequestId, const XtError& error);
        // /客户端用户登出的回调函数
        virtual void onUserLogout(const char* userName, const char* password, int nRequestId, const XtError& error);

        // 下单的回调
        virtual void onOrder(int nRequestId, int orderID, const XtError& error);
        // 撤单的回调
        virtual void onCancel(int nRequestId, const XtError& error);
        // 撤委托的回调
//        virtual void onCancelOrder(int nRequestId,  const XtError& error);
        // 风险试算的回调
//        virtual void onCheck(int nRequestId ,const CCheckData* data , const XtError& error);

        // 请求当前数据的回调函数
        // @param data 请求数据的返回
        // @param isLast 请求数据可能有多条，需要多次回调该函数，标记是否是一次请求的最后一次回调
        virtual void onReqAccountDetail(const char* accountID, int nRequestId, const CAccountDetail* data, bool isLast, const XtError& error);
//        virtual void onReqCreditAccountDetail(const char* accountID, int nRequestId, const CCreditAccountDetail* data, bool isLast, const XtError& error);
        virtual void onReqOrderDetail(const char* accountID, int nRequestId, const COrderDetail* data, bool isLast, const XtError& error);
        virtual void onReqDealDetail(const char* accountID, int nRequestId, const CDealDetail* data, bool isLast, const XtError& error);
//        virtual void onReqPositionDetail(const char* accountID, int nRequestId, const CPositionDetail* data, bool isLast, const XtError& error);
//        virtual void onReqPositionStatics(const char* accountID, int nRequestId, const CPositionStatics* data, bool isLast, const XtError& error);

//        virtual void onReqStksubjects(const char* accountID, int nRequestId, const CStkSubjects* data, bool isLast, const XtError& error);
//        virtual void onReqStkcompacts(const char* accountID, int nRequestId, const CStkCompacts* data, bool isLast, const XtError& error);
//        virtual void onReqCoveredStockPosition(const char* accountID, int nRequestId, const CCoveredStockPosition* data, bool isLast, const XtError& error);
//        virtual void onReqProductData(int nRequestId, const CProductData* data, bool isLast, const XtError& error);
        virtual void onReqCInstrumentDetail(const char* accountID, int nRequestId, const CInstrumentDetail* data, bool isLast, const XtError& error);

        // 主推接口
        // 获得主推的用户登录状态
        // @param   status 主推资金账号的登录状态
        // @param   brokerType 主推资金账号的类型 
        // 1:期货账号, 2:股票账号, 3:信用账号, 4:贵金属账号, 5:期货期权账号, 6:股票期权账号, 7:沪港通账号, 10:全国股转账号
        virtual void onRtnLoginStatus(const char* accountID, EBrokerLoginStatus status, int brokerType, const char* errorMsg);
        // 获取主推的报单状态（指令）
        virtual void onRtnOrder(const COrderInfo* data);
        // 获得主推的委托明细（委托）
        virtual void onRtnOrderDetail(const COrderDetail* data);
        // 获得主推的成交明细
        virtual void onRtnDealDetail(const CDealDetail* data);
        // 获得主推的委托错误信息
        virtual void onRtnOrderError(const COrderError* data);
        // 获得主推的撤销信息
        virtual void onRtnCancelError(const CCancelError* data);

protected:
    int 					curAccountIdx_ 	= 0;
    vector<AccountUnitXt> 	account_units_;
    string 					name_;
    long					timout_ns_				= 5000000000L;
    bool					query_complete_flag_ 	= false;
    unordered_map<int, int> order2request_;
    json					config_;
};

typedef CTDEngineXt* (*tfpCreateTDEngineXt)();

extern "C" CTDEngineXt* createTDEngineXt();

#endif /* TD_CTDENGINECTP_H_ */
