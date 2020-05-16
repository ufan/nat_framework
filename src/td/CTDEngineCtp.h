/*
 * CTDEngineCtp.h
 *
 *  Created on: 2018年4月29日
 *      Author: sky
 *
 *  Note: Copy from Kungfu
 */

#ifndef TD_CTDENGINECTP_H_
#define TD_CTDENGINECTP_H_

#include <vector>
#include <string>
#include <stdint.h>
#include "ITDEngine.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
using namespace std;

/**
 * account information unit extra for CTP is here.
 */
struct AccountUnitCTP
{
  CThostFtdcInputOrderField 			input_order; // order insert
  CThostFtdcInputOrderActionField		action_order; // order action

  CThostFtdcTraderApi* api; // trader api instance, with a thread inside

  // extra information
  int     front_id 	= -1;
  int     session_id 	= -1;

  // internal flags
  bool    initialized;
  bool    connected;
  bool    authenticated;
  bool    settle_confirmed;
  bool    logged_in;

  // some rids, 'rid' means 'nRequestID' in CTP
  int     settle_rid;
  int     login_rid;

  string broker_id;
  string user_id;
  string passwd;
	string user_product_info;
};

class CTDEngineCtp: public ITDEngine, public CThostFtdcTraderSpi
{
 public:
	CTDEngineCtp();
	virtual ~CTDEngineCtp();

	virtual string name() {return name_;}

  // 1. Read front address, accounts info from json file
  // 2. Setup an TradeerApi instance for each account configured, all TraderApi share this Spi instance
  // 3. Connect to trader front and login for each account
  // 4. Fail on any one connection error or one login error.
	virtual bool init(const json& j_conf);

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

 public:
  virtual bool is_connected() const;
  virtual bool is_logged_in() const;
  virtual void req_order_insert(const tIOInputOrderField* data);
  virtual void req_order_action(const tIOrderAction* data);
  virtual bool getBaseInfo();

 public:
	virtual void OnFrontConnected();
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

 protected:
	long					int_user_id_ 	= 0; // from json config key 'myid_8b'
	string				user_product_info; // a copy of json config key 'myid_8b'
  int 					curAccountIdx_ 	= 0; // current account index manipulated
  vector<AccountUnitCTP> 	account_units_; // collection of accounts managed, multiple accounts possible
  string				front_uri_; // front address
  string 				name_; // name of the engine instance
  long					timout_ns_		= 5000000000L;
  bool					query_complete_flag_ = false;
  int 					max_ref_id_ = 0;
};

typedef CTDEngineCtp* (*tfpCreateTDEngineCTP)();

extern "C" CTDEngineCtp* createTDEngineCTP();


#endif /* TD_CTDENGINECTP_H_ */


