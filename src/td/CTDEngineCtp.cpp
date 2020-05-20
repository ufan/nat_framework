/*
 * CTDEngineCtp.cpp
 *
 *  Created on: 2018年4月29日
 *      Author: sky
 */

#include "CTDEngineCtp.h"
#include "Logger.h"
#include "CTimer.h"
#include "CEncodeConv.h"
#include "utils.h"
#include "IOCommon.h"
#include "compiler.h"

#define CHK_USERID(p) (*(long*)(&(p->UserID)) == int_user_id_)

CTDEngineCtp::CTDEngineCtp()
{

}

CTDEngineCtp::~CTDEngineCtp()
{

}

bool CTDEngineCtp::init(const json& j_conf)
{
	json ctp_conf = j_conf["CTPTD"];

  // engine name
	name_ = ctp_conf["name"];

  // self-defined product info, needed in user login
	string myid = ctp_conf["myid_8b"];
	strncpy((char*)&int_user_id_, myid.c_str(), sizeof(int_user_id_));
	user_product_info = myid;

	max_ref_id_ = request_id_start_ - 1;

  // front address
	front_uri_ = ctp_conf["front_uri"];

  // AuthCode and AppID, all accounts share the same
  auth_code_ = ctp_conf["AuthCode"];
  app_id_ = ctp_conf["AppID"];

  // init accounts to be managed
	auto& acc_json = ctp_conf["Account"];
	account_units_.resize(acc_json.size());
	for(int i = 0; i < acc_json.size(); i++)
	{
		load_account(i, acc_json[i]);
	}

  // timeout value
	timout_ns_ = ctp_conf["timeout"].get<long>() * 1000000000L;

  // create trade flow directory
	string trade_flow_path = ctp_conf["trade_flow_path"];
	if(!createPath(trade_flow_path.data())){
		LOG_ERR("create dir %s err", trade_flow_path.data());
		return false;
	}

  // connect to front for each account
	connect(timout_ns_, trade_flow_path);
	if(!is_connected())
	{
		ALERT("connect to ctp timeout.");
		return false;
	}

  // login for each account
	login(timout_ns_);
	if(!is_logged_in())
	{
		ALERT("login ctp timeout.");
		return false;
	}
	return true;
}

bool CTDEngineCtp::load_account(int idx, const json& j_config)
{
    // internal load
    string broker_id = j_config["BrokerID"].get<string>();
    string user_id = j_config["UserID"].get<string>();
    string password = j_config["Password"].get<string>();

    AccountUnitCTP& unit = account_units_[idx];
    unit.broker_id = broker_id;
    unit.user_id = user_id;
    unit.passwd = password;
    unit.user_product_info = user_product_info;
    unit.api = nullptr;
    unit.front_id = -1;
    unit.session_id = -1;
    unit.initialized = false;
    unit.connected = false;
    unit.authenticated = false;
    unit.logged_in = false;
    unit.settle_confirmed = false;

    // init insert order struct
    CThostFtdcInputOrderField &order = unit.input_order;
    memset(&order, 0, sizeof(order));
    strcpy(order.BrokerID, broker_id.c_str());
    strcpy(order.InvestorID, user_id.c_str());
    order.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    order.ContingentCondition = THOST_FTDC_CC_Immediately;
    order.VolumeCondition = THOST_FTDC_VC_AV;
    order.MinVolume = 1;
    order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    order.IsAutoSuspend = 0;
    order.UserForceClose = 0;
    order.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    order.TimeCondition = THOST_FTDC_TC_GFD;
    *(long*)&(order.UserID) = int_user_id_;
    order.UserID[sizeof(int_user_id_)] = '\0';

    // init action order struct
    CThostFtdcInputOrderActionField &action_order = unit.action_order;
    memset(&action_order, 0, sizeof(action_order));
    strcpy(action_order.BrokerID, broker_id.c_str());
    strcpy(action_order.InvestorID, user_id.c_str());
    action_order.ActionFlag = THOST_FTDC_AF_Delete;
    *(long*)&(action_order.UserID) = int_user_id_;
    action_order.UserID[sizeof(int_user_id_)] = '\0';

    return true;
}

void CTDEngineCtp::connect(long timeout_nsec, string trade_flow_path)
{
  for (int idx = 0; idx < account_units_.size(); idx ++)
  {
    AccountUnitCTP& unit = account_units_[idx];
    if (unit.api == nullptr)
    {
      CThostFtdcTraderApi* api = CThostFtdcTraderApi::CreateFtdcTraderApi(trade_flow_path.c_str());
      if (!api)
      {
        throw std::runtime_error("CTP_TD failed to create api");
      }
      api->RegisterSpi(this);
      unit.api = api;
    }
    if (!unit.connected)
    {
      curAccountIdx_ = idx;
      unit.api->RegisterFront((char*)front_uri_.c_str());
      unit.api->SubscribePublicTopic(THOST_TERT_QUICK); // need check
      unit.api->SubscribePrivateTopic(THOST_TERT_QUICK); // need check
      if (!unit.initialized)
      {
        unit.api->Init();
        unit.initialized = true;
      }
      long start_time = CTimer::instance().getNano();
      while (!unit.connected && CTimer::instance().getNano() - start_time < timeout_nsec)
      {usleep(50000);}
    }
  }
}

// Send login request and settle request
// The function works in Sync mode, i.e. it will wait until request sucess or timeout
void CTDEngineCtp::login(long timeout_nsec)
{
  for (int idx = 0; idx < account_units_.size(); idx ++)
  {
    AccountUnitCTP& unit = account_units_[idx];

    // authenticate
    if (!unit.authenticated)
    {
      // send out authenticate request
      CThostFtdcReqAuthenticateField reqAuth;
      memset(&reqAuth, 0, sizeof(reqAuth));
      strcpy(reqAuth.BrokerID, unit.broker_id.c_str());
      strcpy(reqAuth.UserID, unit.user_id.c_str());
      strcpy(reqAuth.UserProductInfo, unit.user_product_info.c_str()); // it's not necessary
      strcpy(reqAuth.AuthCode, auth_code_.c_str());
      strcpy(reqAuth.AppID, app_id_.c_str());
      unit.auth_rid = request_id_;

      if (unit.api->ReqAuthenticate(&reqAuth, request_id_++))
      {
        ALERT("[request] authenticate failed! (Bid)%s (Uid)%s", reqAuth.BrokerID, reqAuth.UserID);
      }

      // wait for response until timeout
      long start_time = CTimer::instance().getNano();
      while (!unit.authenticated && CTimer::instance().getNano() - start_time < timeout_nsec)
      {
        usleep(50000);
      }
    }

    // login
    if (!unit.logged_in)
    {
      // send out login request
      struct CThostFtdcReqUserLoginField req = {};
      strcpy(req.TradingDay, "");
      strcpy(req.UserID, unit.user_id.c_str());
      strcpy(req.BrokerID, unit.broker_id.c_str());
      strcpy(req.Password, unit.passwd.c_str());
			strcpy(req.UserProductInfo, unit.user_product_info.c_str());
      unit.login_rid = request_id_;

      if (unit.api->ReqUserLogin(&req, request_id_++))
      {
        ALERT("[request] login failed! (Bid)%s (Uid)%s", req.BrokerID, req.UserID);
      }

      // wait for login response until timeout
      long start_time = CTimer::instance().getNano();
      while (!unit.logged_in && CTimer::instance().getNano() - start_time < timeout_nsec)
      {
        usleep(50000);
      }
    }

    // confirm settlement
    if (!unit.settle_confirmed)
    {
      // send out SettlementConfirm request
      struct CThostFtdcSettlementInfoConfirmField req = {};
      strcpy(req.BrokerID, unit.broker_id.c_str());
      strcpy(req.InvestorID, unit.user_id.c_str());
      unit.settle_rid = request_id_;

      if (unit.api->ReqSettlementInfoConfirm(&req, request_id_++))
      {
        ALERT("[request] settlement info failed! (Bid)%s (Iid)%s", req.BrokerID, req.InvestorID);
      }

      // wait for settlement confirm until timeout
      long start_time = CTimer::instance().getNano();
      while (!unit.settle_confirmed && CTimer::instance().getNano() - start_time < timeout_nsec)
      {
        usleep(50000);
      }
    }
  }
}

// send logout request for each account to front
void CTDEngineCtp::logout()
{
  for (int idx = 0; idx < account_units_.size(); idx++)
  {
    AccountUnitCTP& unit = account_units_[idx];
    if (unit.logged_in)
    {
      CThostFtdcUserLogoutField req = {};
      strcpy(req.BrokerID, unit.broker_id.c_str());
      strcpy(req.UserID, unit.user_id.c_str());
      unit.login_rid = request_id_;

      if (unit.api->ReqUserLogout(&req, request_id_++))
      {
        ALERT("[request] logout failed! (Bid)%s (Uid)%s", req.BrokerID, req.UserID);
      }
    }
    unit.authenticated = false;
    unit.settle_confirmed = false;
    unit.logged_in = false;
  }
}

// Query all the instrument information.
bool CTDEngineCtp::queryInstruments(long timeout_nsec)
{
	int ret = 0;
	CThostFtdcQryInstrumentField req;
  // empty field means query all instrument information
	memset(&req, 0, sizeof(req));
  // All accounts share the same set of instrument information, so only need to query once
	if(0 != (ret = account_units_[0].api->ReqQryInstrument(&req, request_id_++)))
	{
		ALERT("[request] ReqQryInstrument err:%d, ", ret);
		return false;
	}

  // Waiting until timeout
  long start_time = CTimer::instance().getNano();
  while (!CTradeBaseInfo::is_init_ && CTimer::instance().getNano() - start_time < timeout_nsec) {
    usleep(50000);
  }

  // If not all instruments returned, fail
  if(!CTradeBaseInfo::is_init_)
  {
    ALERT("[request] ReqQryInstrument timeout");
    return false;
  }
	return true;
}

void CTDEngineCtp::release_api()
{
  for (auto& unit: account_units_)
  {
    if (unit.api != nullptr)
    {
      unit.api->Release();
      unit.api = nullptr;
    }
    unit.initialized = false;
    unit.connected = false;
    unit.authenticated = false;
    unit.settle_confirmed = false;
    unit.logged_in = false;
    unit.api = nullptr;
  }
}

// check wether all accounts are login and settled
bool CTDEngineCtp::is_logged_in() const
{
  for (auto& unit: account_units_)
  {
    if (!unit.authenticated || !unit.logged_in || !unit.settle_confirmed)
      return false;
  }
  return true;
}

// check whether all accounts' TraderApi are connected
bool CTDEngineCtp::is_connected() const
{
  for (auto& unit: account_units_)
  {
    if (!unit.connected)
      return false;
  }
  return true;
}

// Response for success connection
void CTDEngineCtp::OnFrontConnected()
{
    ENGLOG("[OnFrontConnected] (idx)%d", curAccountIdx_);
    account_units_[curAccountIdx_].connected = true;
}

void CTDEngineCtp::OnFrontDisconnected(int nReason)
{
	ALERT("[OnFrontDisconnected] reason=%d", nReason);
  for (auto& unit: account_units_)
  {
    unit.connected = false;
    unit.authenticated = false;
    unit.settle_confirmed = false;
    unit.logged_in = false;
  }
  stop();
}

// Callback on authenticate request
void CTDEngineCtp::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
  // check whether rid is in valid range
	if(unlikely(!checkRequestId(nRequestID))) return;

  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
  {
    ALERT("[OnRspAuthenticate] (errId)%d (errMsg)%s", pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  }
  else{
    // print out authenticate response
    ENGLOG("[OnRspAuthenticate] (Bid)%s (Uid)%s (AppId)%s (AppType)%d (ProInfo)%s",
           pRspAuthenticateField->BrokerID,
           pRspAuthenticateField->UserID,
           pRspAuthenticateField->AppID,
           pRspAuthenticateField->AppType,
           pRspAuthenticateField->UserProductInfo
           );

    // find the corresponding account matching this rid
    for (auto& unit: account_units_)
    {
      if (unit.auth_rid == nRequestID)
      {
        unit.authenticated = true;
      }
    }
  }
}

void CTDEngineCtp::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                 int nRequestID, bool bIsLast)
{
  // check whether rid is in valid range
	if(unlikely(!checkRequestId(nRequestID))) return;

  // check whether the request is successful
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
  {
    ALERT("[OnRspUserLogin] (errId)%d (errMsg)%s", pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  }
  else // User Login Succeed!
  {
    ENGLOG("[OnRspUserLogin] (Bid)%s (Uid)%s (maxRef)%s (Fid)%d (Sid)%d", pRspUserLogin->BrokerID,
           pRspUserLogin->UserID, pRspUserLogin->MaxOrderRef, pRspUserLogin->FrontID, pRspUserLogin->SessionID);

    // find the corresponding account matching this rid
    for (auto& unit: account_units_)
    {
      if (unit.login_rid == nRequestID)
      {
        unit.action_order.FrontID = unit.front_id = pRspUserLogin->FrontID;
        unit.action_order.SessionID = unit.session_id = pRspUserLogin->SessionID;
        unit.logged_in = true;
      }
    }
    int max_ref = atoi(pRspUserLogin->MaxOrderRef) + 1;
    request_id_ = (max_ref > request_id_) ? max_ref: request_id_;
  }
}

void CTDEngineCtp::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
  // check whether rid is in valid range
	if(unlikely(!checkRequestId(nRequestID))) return;

  // check whether the request is successful
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
  {
    ALERT("[OnRspSettlementInfoConfirm] (errId)%d (errMsg)%s", pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  }
  else // settlement confirmed successfully
  {
    ENGLOG("[OnRspSettlementInfoConfirm] (brokerID)%s (investorID)%s (confirmDate)%s (confirmTime)%s", pSettlementInfoConfirm->BrokerID,
           pSettlementInfoConfirm->InvestorID, pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);

    // find the corresponding account matching this rid
    for (auto& unit: account_units_)
    {
      if (unit.settle_rid == nRequestID)
      {
        unit.settle_confirmed = true;
      }
    }
  }
}

void CTDEngineCtp::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                  int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;

  if (pRspInfo != nullptr && pRspInfo->ErrorID == 0)
  {
    ALERT("[OnRspUserLogout] (errId)%d (errMsg)%s", pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  }
  else
  {
    ENGLOG("[OnRspUserLogout] (brokerId)%s (userId)%s", pUserLogout->BrokerID, pUserLogout->UserID);
    for (auto& unit: account_units_)
    {
      if (unit.login_rid == nRequestID)
      {
        unit.logged_in = false;
        unit.authenticated = false;
        unit.settle_confirmed = false;
      }
    }
  }
}

// Push the returned instrument information into TradeBaseInfo.
// Each invokation of the callback means insertion of one new instrument.
void CTDEngineCtp::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;
	if(pRspInfo && pRspInfo->ErrorID != 0)
	{
		string err = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
		ALERT("CTP OnRspQryInstrument err:%d, msg:%s", pRspInfo->ErrorID, err.c_str());
	}
	else
	{
		if(pInstrument)
		{
			uint32_t hash = INSTR_NAME_TO_HASH(pInstrument->InstrumentID);
			tInstrumentInfo &info = CTradeBaseInfo::instr_info_[hash];
			info.instr_hash = hash;
			memcpy(info.instr, pInstrument->InstrumentID, sizeof(info.instr));
			info.exch = exchangeStr2int(pInstrument->ExchangeID);
			memcpy(info.product, pInstrument->ProductID, sizeof(info.product));
			info.product_hash = INSTR_NAME_TO_HASH(info.product);
			info.vol_multiple = pInstrument->VolumeMultiple;
			info.tick_price = pInstrument->PriceTick;
			memcpy(info.expire_date, pInstrument->ExpireDate, sizeof(info.expire_date));
			info.is_trading = pInstrument->IsTrading;
      ENGLOG("[OnRspQryInstrument] (InstrID)%s (ProdID)%s (PriceTick)%f",
             pInstrument->InstrumentID, pInstrument->ProductID, info.tick_price);
		}
		if(bIsLast)
		{
			CTradeBaseInfo::is_init_ = true;
			ENGLOG("OnRspQryInstrument finished.");
		}
	}
}

void CTDEngineCtp::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
                                    int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;
	tOrderTrack& request_track = get_request_track(nRequestID);
	if(pRspInfo && request_track.from != 0 && CHK_USERID(pInputOrder))
	{
		if(request_track.status & ODS(REJECT)) return;
		request_track.status |= ODS(REJECT);

		tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
		p->cmd = IO_ORDER_RTN;
		p->to = request_track.from;
		p->rtn_msg.msg_type = ODS(REJECT);
		p->rtn_msg.local_id = request_track.local_id;
		memcpy(p->rtn_msg.instr, pInputOrder->InstrumentID, sizeof(pInputOrder->InstrumentID));
		p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
		p->rtn_msg.price = request_track.price;
		p->rtn_msg.vol = request_track.vol;
		p->rtn_msg.dir = request_track.dir;
		p->rtn_msg.off = request_track.off;
		p->rtn_msg.order_ref = nRequestID;
		p->rtn_msg.front_id = request_track.front_id;
		p->rtn_msg.session_id = request_track.session_id;

		p->rtn_msg.errid = pRspInfo->ErrorID;
		strncpy(p->rtn_msg.msg, pRspInfo->ErrorMsg, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

void CTDEngineCtp::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;
	tOrderTrack& request_track = get_request_track(nRequestID);
	if(pRspInfo && request_track.from != 0 && CHK_USERID(pInputOrderAction))
	{
		request_track.status |= ODS(CANCEL_REJECT);

		tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
		p->cmd = IO_ORDER_RTN;
		p->to = request_track.from;
		p->rtn_msg.msg_type = ODS(CANCEL_REJECT);
		p->rtn_msg.local_id = request_track.local_id;
		memcpy(p->rtn_msg.instr, pInputOrderAction->InstrumentID, sizeof(pInputOrderAction->InstrumentID));
		p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
		p->rtn_msg.price = request_track.price;
		p->rtn_msg.vol = request_track.vol;
		p->rtn_msg.dir = request_track.dir;
		p->rtn_msg.off = request_track.off;
		p->rtn_msg.order_ref = nRequestID;
		p->rtn_msg.front_id = request_track.front_id;
		p->rtn_msg.session_id = request_track.session_id;

		p->rtn_msg.errid = pRspInfo->ErrorID;
		strncpy(p->rtn_msg.msg, pRspInfo->ErrorMsg, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

void CTDEngineCtp::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	if(unlikely(!CHK_USERID(pOrder))) return;
	int request_id = atoi(pOrder->OrderRef);
	if(unlikely(!checkRequestId(request_id))) return;
	tOrderTrack& request_track = get_request_track(request_id);

	tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
	p->cmd = IO_ORDER_RTN;
	p->to = request_track.from;
	p->rtn_msg.local_id = request_track.local_id;
	memcpy(p->rtn_msg.instr, pOrder->InstrumentID, sizeof(pOrder->InstrumentID));
	p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
	p->rtn_msg.price = request_track.price;
	p->rtn_msg.vol = request_track.vol;
	p->rtn_msg.dir = request_track.dir;
	p->rtn_msg.off = request_track.off;
	p->rtn_msg.order_ref = request_id;
	p->rtn_msg.front_id = request_track.front_id;
	p->rtn_msg.session_id = request_track.session_id;

	switch(pOrder->OrderStatus)
	{
	case THOST_FTDC_OST_Unknown:
		if(request_track.status & ODS(ACCEPT))
		{
			writer_.discard();
			return;
		}
		request_track.status |= ODS(ACCEPT);
		p->rtn_msg.msg_type = ODS(ACCEPT);
		break;

	case THOST_FTDC_OST_Canceled:
	case THOST_FTDC_OST_PartTradedNotQueueing:
		if(request_track.status & ODS(CANCELED))
		{
			writer_.discard();
			return;
		}
		request_track.status |= ODS(CANCELED);
		p->rtn_msg.msg_type = ODS(CANCELED);
		p->rtn_msg.vol = pOrder->VolumeTotal;
		strncpy(p->rtn_msg.msg, pOrder->StatusMsg, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		break;

	case THOST_FTDC_OST_AllTraded:
		request_track.status |= ODS(CLOSED);
		/* no break */
	case THOST_FTDC_OST_NoTradeQueueing:
	case THOST_FTDC_OST_PartTradedQueueing:
		if(request_track.status & ODS(MARKET_ACCEPT))
		{
			writer_.discard();
			return;
		}
		request_track.status |= ODS(MARKET_ACCEPT);
		p->rtn_msg.msg_type = ODS(MARKET_ACCEPT);
		break;
	}
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineCtp::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if(unlikely(!CHK_USERID(pTrade))) return;
	int request_id = atoi(pTrade->OrderRef);
	if(unlikely(!checkRequestId(request_id))) return;
	if(unlikely(testOtId(request_id, strtoll(pTrade->TradeID, NULL, 0)))) return;
	tOrderTrack& request_track = get_request_track(request_id);
	if(request_track.from != 0)
	{
		request_track.status |= ODS(EXECUTION);
		request_track.vol_traded += pTrade->Volume;
		if(request_track.vol_traded >= request_track.vol) request_track.status |= ODS(CLOSED);

		tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
		p->cmd = IO_ORDER_RTN;
		p->to = request_track.from;
		p->rtn_msg.msg_type = ODS(EXECUTION);
		p->rtn_msg.local_id = request_track.local_id;
		memcpy(p->rtn_msg.instr, pTrade->InstrumentID, sizeof(pTrade->InstrumentID));
		p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
		p->rtn_msg.price = pTrade->Price;
		p->rtn_msg.vol = pTrade->Volume;
		p->rtn_msg.dir = pTrade->Direction;
		p->rtn_msg.off = pTrade->OffsetFlag;
		p->rtn_msg.order_ref = request_id;
		p->rtn_msg.front_id = request_track.front_id;
		p->rtn_msg.session_id = request_track.session_id;
		writer_.commit();

		request_track.amount_traded += pTrade->Price * pTrade->Volume;
		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

void CTDEngineCtp::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if(unlikely(!CHK_USERID(pInputOrder))) return;
	int request_id = atoi(pInputOrder->OrderRef);
	if(unlikely(!checkRequestId(request_id))) return;
	tOrderTrack& request_track = get_request_track(request_id);
	if(request_track.from != 0)
	{
		if(request_track.status & ODS(MARKET_REJECT)) return;
		request_track.status |= ODS(MARKET_REJECT);

		tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
		p->cmd = IO_ORDER_RTN;
		p->to = request_track.from;
		p->rtn_msg.msg_type = ODS(MARKET_REJECT);
		p->rtn_msg.local_id = request_track.local_id;
		memcpy(p->rtn_msg.instr, pInputOrder->InstrumentID, sizeof(pInputOrder->InstrumentID));
		p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
		p->rtn_msg.price = request_track.price;
		p->rtn_msg.vol = request_track.vol;
		p->rtn_msg.dir = request_track.dir;
		p->rtn_msg.off = request_track.off;
		p->rtn_msg.order_ref = request_id;
		p->rtn_msg.front_id = request_track.front_id;
		p->rtn_msg.session_id = request_track.session_id;
		p->rtn_msg.errid = pRspInfo->ErrorID;
		strncpy(p->rtn_msg.msg, pRspInfo->ErrorMsg, sizeof(p->rtn_msg.msg) - 1);
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

void CTDEngineCtp::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if(unlikely(!CHK_USERID(pOrderAction))) return;
	int request_id = atoi(pOrderAction->OrderRef);
	if(unlikely(!checkRequestId(request_id))) return;
	tOrderTrack& request_track = get_request_track(request_id);
	if(request_track.from != 0)
	{
		request_track.status |= ODS(CANCEL_REJECT);

		tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
		p->cmd = IO_ORDER_RTN;
		p->to = request_track.from;
		p->rtn_msg.msg_type = ODS(CANCEL_REJECT);
		p->rtn_msg.local_id = request_track.local_id;
		memcpy(p->rtn_msg.instr, pOrderAction->InstrumentID, sizeof(pOrderAction->InstrumentID));
		p->rtn_msg.instr_hash = INSTR_NAME_TO_HASH(p->rtn_msg.instr);
		p->rtn_msg.price = request_track.price;
		p->rtn_msg.vol = request_track.vol;
		p->rtn_msg.dir = request_track.dir;
		p->rtn_msg.off = request_track.off;
		p->rtn_msg.order_ref = request_id;
		p->rtn_msg.front_id = request_track.front_id;
		p->rtn_msg.session_id = request_track.session_id;
		p->rtn_msg.errid = pRspInfo->ErrorID;
		strncpy(p->rtn_msg.msg, pRspInfo->ErrorMsg, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

// Request OrderInsert to front
// order_ref = request_id_ = trackOrder index
void CTDEngineCtp::req_order_insert(const tIOInputOrderField* data)
{
	//elapse_begin(start_order);
	if((uint32_t)(data->acc_idx) < acc_utilis_.size())
	{
		auto& util = acc_utilis_[data->acc_idx];
		int off = data->off;
		int ret = 0;
    // checking the risk first
		if(unlikely(ret = util->check(data->dir, off, data->price, data->vol, data->instr_hash)))
		{
			writeErrRtn(data, ret, "risk check failed");
		}
		else
		{
      // find the corresponding account
			AccountUnitCTP& unit = account_units_[data->acc_idx];
			auto& order = unit.input_order;
			int2string(order.OrderRef, request_id_); // order_ref comes from current request_id_
			strcpy(order.InstrumentID, data->instr);
			order.VolumeTotalOriginal = data->vol;
			order.CombOffsetFlag[0] = off;
			order.Direction = data->dir;
			order.LimitPrice = data->price;

      // send the request to matching trader front
			//elapse_begin(call_api);
			int ret = unit.api->ReqOrderInsert(&order, request_id_);
			if(ret == 0)	// succ, then request_id_ should ++
			{
        // insert this order into request tracked order
				//elapse_begin(call_api_after);
				tOrderTrack& request_track = get_request_track(request_id_);
				request_track.status = ODS(TDSEND);
				request_track.instr_hash = data->instr_hash;
				memcpy(request_track.instr, data->instr, sizeof(request_track.instr));
				request_track.price = data->price;
				request_track.vol = data->vol;
				request_track.dir = data->dir;
				request_track.off = data->off;
				request_track.vol_traded = 0;
				request_track.from = data->from;
				request_track.local_id = data->local_id;
				request_track.acc_id = data->acc_idx;
				request_track.order_ref = request_id_;
				request_track.front_id = unit.front_id;
				request_track.session_id = unit.session_id;
				request_track.stg_id = data->stg_id;

				// write ack back
				tIOrderRtn *p = (tIOrderRtn*)writer_.prefetch(sizeof(tIOrderRtn));
				p->cmd = IO_ORDER_RTN;
				p->to = data->from;
				p->rtn_msg.msg_type = ODS(TDSEND);
				p->rtn_msg.local_id = data->local_id;
				p->rtn_msg.instr_hash = data->instr_hash;
				memcpy(p->rtn_msg.instr, data->instr, sizeof(p->rtn_msg.instr));
				p->rtn_msg.price = data->price;
				p->rtn_msg.vol = data->vol;
				p->rtn_msg.dir = data->dir;
				p->rtn_msg.off = data->off;
				p->rtn_msg.order_ref = request_id_++; // request_id_ increment here, as no more usage in scope
				p->rtn_msg.front_id = request_track.front_id;
				p->rtn_msg.session_id = request_track.session_id;
				writer_.commit();
				
				//elapse_begin(committed);
				//LOG_DBG("elapse:%ld,%ld,%ld,%ld", start_order - getIOFrameHead(data)->nano, call_api - start_order, call_api_after - call_api, committed - call_api_after);

				util->onNew(data->dir, off, data->price, data->vol, data->instr_hash, request_track.order_ref);
			}
			else
			{
				writeErrRtn(data, ret, "ctp ReqOrderInsert api failed.");
			}
		}
	}
	else
	{
		writeErrRtn(data, -1, "account idx err.");
	}
}

void CTDEngineCtp::req_order_action(const tIOrderAction* data)
{
	if(unlikely(!checkRequestId(data->order_ref)))
	{
		writeErrRtn(data, -1, "cancel order reject by TDEngine: order_ref range err.", ODS(CANCEL_REJECT));
		return;
	}
	tOrderTrack& request_track = get_request_track(data->order_ref);
	if(request_track.front_id == data->front_id && request_track.session_id == data->session_id)
	{
		AccountUnitCTP& unit = account_units_[data->acc_idx];
		auto& order = unit.action_order;
		int2string(order.OrderRef, data->order_ref);
		order.FrontID = data->front_id;
		order.SessionID = data->session_id;
		strcpy(order.InstrumentID, data->instr);

		int ret = unit.api->ReqOrderAction(&order, data->order_ref);	// use order_ref as request_id
		if(ret == 0)
		{
			request_track.status |= ODS(CXLING);
		}
		else
		{
			writeErrRtn(data, ret, "ctp ReqOrderAction api failed.", ODS(CANCEL_REJECT));
		}
	}
	else
	{
		writeErrRtn(data, -1, "cancel order reject by TDEngine.", ODS(CANCEL_REJECT));
	}
}

// Init the CTradeBaseInfo
bool CTDEngineCtp::getBaseInfo()
{
  // Get current trading day from front
	CTradeBaseInfo::trading_day_ = account_units_[0].api->GetTradingDay();
  // Get all the listed instruments from front and push them into TradeBaseInfo
	return queryInstruments(timout_ns_);
}

void CTDEngineCtp::release()
{
	logout();
	release_api();
}

// Update the tracked orders
bool CTDEngineCtp::updateOrderTrack()
{
  // ctp limit request frequency, so sleep 1s here.
	usleep(1000000);

  // reset the order track mmap buffer if a new trading day begins
	if(CTradeBaseInfo::trading_day_ != otmmap_.getBuf()->trading_day)
  {
    otmmap_.clearTrack();
  }

  // get the current trading day
	strcpy(otmmap_.getBuf()->trading_day, CTradeBaseInfo::trading_day_.c_str());

  // query the orders for each account
  for (int idx = 0; idx < account_units_.size(); idx ++)
  {
    query_complete_flag_ = false;
    curAccountIdx_ = idx;
    AccountUnitCTP& unit = account_units_[idx];
		CThostFtdcQryOrderField query;
		memset(&query, 0, sizeof(query));
		strcpy(query.BrokerID, unit.broker_id.c_str());
		strcpy(query.InvestorID, unit.user_id.c_str());

		if (int ret = unit.api->ReqQryOrder(&query, request_id_++))
		{
			ALERT("ReqQryOrder failed: %d", ret);
			return  false;
		}

    // waiting for the response until timeout
		long start_time = CTimer::instance().getNano();
		while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timout_ns_)
		{usleep(50000);}

		if(!query_complete_flag_) return false;
  }
  request_id_ = max_ref_id_ + 1;

	if(!updateTradedAmount())
	{
		ALERT("updateTradedAmount failed.");
		return false;
	}
  return true;
}

void CTDEngineCtp::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;

	if(bIsLast) query_complete_flag_ = true;

	if(pOrder)
	{
		if(unlikely(!CHK_USERID(pOrder))) return;

		int order_ref = atoi(pOrder->OrderRef);
		if(unlikely(!checkRequestId(order_ref))) return;

		tOrderTrack& track = get_request_track(order_ref);
		pOrder->OrderStatus = ODS(TDSEND);
		switch(pOrder->OrderStatus)
		{
		case THOST_FTDC_OST_Unknown:
			track.status |= ODS(ACCEPT);
			break;

		case THOST_FTDC_OST_Canceled:
		case THOST_FTDC_OST_PartTradedNotQueueing:
			track.status |= ODS(CANCELED);
			break;

		case THOST_FTDC_OST_NoTradeQueueing:
			track.status |= ODS(MARKET_ACCEPT);
			break;

		case THOST_FTDC_OST_AllTraded:
			track.status |= ODS(CLOSED);
			/* no break */
		case THOST_FTDC_OST_PartTradedQueueing:
			track.status |= ODS(EXECUTION);
			break;
		}
		memcpy(track.instr, pOrder->InstrumentID, sizeof(track.instr));
		track.instr_hash = INSTR_NAME_TO_HASH(track.instr);
		track.price = pOrder->LimitPrice;
		track.vol = pOrder->VolumeTotalOriginal;
		track.dir = pOrder->Direction;
		// track.off = pOrder->CombOffsetFlag[0];			// comment out this line, because this field from ctp may lose some information.
		track.vol_traded = pOrder->VolumeTraded;
		track.acc_id = curAccountIdx_;
		track.order_ref = order_ref;
		track.front_id = pOrder->FrontID;
		track.session_id = pOrder->SessionID;

		max_ref_id_ = (order_ref > max_ref_id_) ? order_ref : max_ref_id_;
	}
}

bool CTDEngineCtp::updateTradedAmount()
{
	usleep(1000000);	// ctp limit request frequency, so sleep 1s here.

	for(int i = 0; i < MMAP_ORDER_TRACK_SIZE; i++)
	{
		 tOrderTrack& ot = get_request_track(i);
		 ot.amount_traded = 0.0;
	}

    for (int idx = 0; idx < account_units_.size(); idx ++)
    {
    	query_complete_flag_ = false;
    	curAccountIdx_ = idx;
        AccountUnitCTP& unit = account_units_[idx];
    	CThostFtdcQryTradeField query;
    	memset(&query, 0, sizeof(query));
		strcpy(query.BrokerID, unit.broker_id.c_str());
		strcpy(query.InvestorID, unit.user_id.c_str());

		if (int ret = unit.api->ReqQryTrade(&query, request_id_))
		{
			ALERT("ReqQryTrade failed: %d", ret);
			return  false;
		}
		long start_time = CTimer::instance().getNano();
		while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timout_ns_)
		{usleep(50000);}
		if(!query_complete_flag_) return false;
    }
    return true;
}

void CTDEngineCtp::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(unlikely(!checkRequestId(nRequestID))) return;
	if(bIsLast) query_complete_flag_ = true;
	if(pTrade)
	{
		if(unlikely(!CHK_USERID(pTrade))) return;
		int order_ref = atoi(pTrade->OrderRef);
		if(unlikely(!checkRequestId(order_ref))) return;
		tOrderTrack& track = get_request_track(order_ref);
		track.amount_traded += pTrade->Price * pTrade->Volume;
	}
}

CTDEngineCtp* createTDEngineCTP()
{
	return new CTDEngineCtp;
}
