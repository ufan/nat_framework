/*
 * CTDEngineXt.cpp
 *
 *  Created on: 2018年4月29日
 *      Author: sky
 */

#include "CTDEngineXt.h"
#include "Logger.h"
#include "CTimer.h"
#include "utils.h"
#include "IOCommon.h"
#include "compiler.h"
#include "CEncodeConv.h"

CTDEngineXt::CTDEngineXt()
{

}

CTDEngineXt::~CTDEngineXt()
{

}

bool CTDEngineXt::init(const json& j_conf)
{
	config_ = j_conf;
	json xt_conf = j_conf["XTTD"];

	name_ = xt_conf["name"];

	auto& acc_json = xt_conf["Account"];
	account_units_.resize(acc_json.size());
	for(int i = 0; i < acc_json.size(); i++)
	{
		load_account(i, acc_json[i]);
	}

	timout_ns_ = xt_conf["timeout"].get<long>() * 1000000000L;		// timeout in seconds
	string ip_port = xt_conf["ip_port"];
	connect(timout_ns_, ip_port);
	if(!is_connected())
	{
		ALERT("connect to xt timeout.");
		return false;
	}

	login(timout_ns_);
	if(!is_logged_in())
	{
		ALERT("login xt timeout.");
		return false;
	}
	return true;
}

bool CTDEngineXt::load_account(int idx, const json& j_config)
{
	AccountUnitXt& unit = account_units_[idx];
	unit.account_id = j_config["FundAccount"];
	unit.user_id = j_config["UserID"];
	unit.passwd = j_config["Password"];
	unit.api = nullptr;
	unit.initialized = false;
	unit.connected = false;
	unit.logged_in = false;

	memset(&unit.input_order, 0, sizeof(COrdinaryOrder));
	strcpy(unit.input_order.m_strAccountID, unit.account_id.c_str());
	unit.input_order.m_dSuperPriceRate = 0;
	unit.input_order.m_ePriceType = EPriceType::PRTP_FIX;
	unit.input_order.m_eHedgeFlag = EHedgeFlagType::HEDGE_FLAG_SPECULATION;
	unit.input_order.m_dOccurBalance = 0;
	unit.input_order.m_eOverFreqOrderMode = EXtOverFreqOrderMode::OFQ_FORBID;

	return true;
}

void CTDEngineXt::connect(long timeout_nsec, string ip_port)
{
	for (int idx = 0; idx < account_units_.size(); idx ++)
	{
		AccountUnitXt& unit = account_units_[idx];
		if (unit.api == nullptr)
		{
			unit.api = XtTraderApi::createXtTraderApi(ip_port.c_str());
			if (!unit.api)
			{
				throw std::runtime_error("Xt_TD failed to create api");
			}
			unit.api->setCallback(this);
			string xt_config = config_["/XTTD/xt_config_dir"_json_pointer];
			if(not unit.api->init(xt_config.c_str()))
			{
				ALERT("Xt_TD api init config err.");
				throw std::runtime_error("Xt_TD api init config err.");
			}
		}
		if (!unit.connected)
		{
			curAccountIdx_ = idx;
			if (!unit.initialized)
			{
				if (unit.api->init())
				{
					unit.initialized = true;
				}
				else
				{
					ALERT("Xt failed to init");
				}
			}
			long start_time = CTimer::instance().getNano();
			while (!unit.connected && CTimer::instance().getNano() - start_time < timeout_nsec)
			{usleep(50000);}
		}
	}
}

void CTDEngineXt::onConnected(bool success, const char* errorMsg) 
{
	if (success)
	{
		ENGLOG("[OnFrontConnected] (idx)%d", curAccountIdx_);
		account_units_[curAccountIdx_].connected = true;
	}
	else
	{
		ALERT("[OnFrontDisconnected] reason=%s", errorMsg);
		for (auto& unit: account_units_)
		{
			unit.connected = false;
			unit.logged_in = false;
		}
	}
}

void CTDEngineXt::login(long timeout_nsec)
{
	for (curAccountIdx_ = 0; curAccountIdx_ < account_units_.size(); curAccountIdx_ ++)
	{
		AccountUnitXt& unit = account_units_[curAccountIdx_];
		// login
		if (!unit.logged_in)
		{
			unit.api->userLogin(unit.user_id.c_str(), unit.passwd.c_str(), request_id_++);
			long start_time = CTimer::instance().getNano();
			while (!unit.logged_in && CTimer::instance().getNano() - start_time < timeout_nsec)
			{usleep(50000);}
		}
	}
}

void CTDEngineXt::onUserLogin(const char* userName, const char* password, int nRequestId, const XtError& error) 
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("[request] login failed! (Uid)%s, (%d)%s", userName, error.errorID(), CEncodeConv::gbk2utf8(string(error.errorMsg())).c_str());
	}
	else
	{
		ENGLOG("[request] %s login succ", userName);
	}
}

void CTDEngineXt::onRtnLoginStatus(const char* accountID, EBrokerLoginStatus status, int brokerType, const char* errorMsg)
{
	AccountUnitXt& unit = account_units_[curAccountIdx_];
	if (status == EBrokerLoginStatus::BROKER_LOGIN_STATUS_OK)
	{
		// 1:期货账号, 2:股票账号, 3:信用账号, 4:贵金属账号, 5:期货期权账号, 6:股票期权账号, 7:沪港通账号, 10:全国股转账号
        ENGLOG("[OnRspUserLogin] (brokerType)%d (accountID)%s", brokerType, accountID);
		if (strcmp(unit.account_id.c_str(), accountID) != 0)
		{
			LOG_WARN("[OnRspUserLogin] different fund account: %s, %s", unit.account_id.c_str(), accountID);
			unit.account_id = string(accountID);
			strcpy(unit.input_order.m_strAccountID, accountID);
		}
		unit.logged_in = true;
	}
	else
	{
        ALERT("[OnRspUserLogin] (errMsg)%s", errorMsg);
	}
}

void CTDEngineXt::logout()
{
	for (int idx = 0; idx < account_units_.size(); idx++)
	{
		AccountUnitXt& unit = account_units_[idx];
		if (unit.logged_in)
		{
			unit.login_rid = request_id_;
			unit.api->userLogout(unit.user_id.c_str(), unit.passwd.c_str(), request_id_++);
		}
	}
}

void CTDEngineXt::onUserLogout(const char* userName, const char* password, int nRequestId, const XtError& error)
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("[request] logout failed! (Uid)%s, (%d)%s", userName, error.errorID(), error.errorMsg());
	}
	else
	{
		ENGLOG("[onUserLogout] (userName)%s", userName);
        for (auto& unit: account_units_)
        {
            if (unit.login_rid == nRequestId)
            {
                unit.logged_in = false;
                break;
            }
        }
	}
}

void CTDEngineXt::join()
{
	for (int idx = 0; idx < account_units_.size(); idx ++)
	{
		AccountUnitXt& unit = account_units_[idx];
		if (unit.api != nullptr)
		{
			unit.api->join();
		}
	}
}

void CTDEngineXt::release_api()
{
    for (auto& unit: account_units_)
    {
        if (unit.api != nullptr)
        {
            unit.api->destroy();
			delete unit.api;
            unit.api = nullptr;
        }
        unit.initialized = false;
        unit.connected = false;
        unit.logged_in = false;
    }
}

void CTDEngineXt::release()
{
	logout();
	release_api();
}

bool CTDEngineXt::is_logged_in() const
{
    for (auto& unit: account_units_)
    {
        if (!unit.logged_in)
            return false;
    }
    return true;
}

bool CTDEngineXt::is_connected() const
{
    for (auto& unit: account_units_)
    {
        if (!unit.connected)
            return false;
    }
    return true;
}

inline EOperationType CTDEngineXt::ConvJarvisOffsetToXtOffset(int dir, int off)
{
	if (dir == AT_CHAR_Buy)
	{
		if (off == AT_CHAR_Open)
		{
			return EOperationType::OPT_OPEN_LONG;
		}
		else if (off == AT_CHAR_CloseYesterday)
		{
			return EOperationType::OPT_CLOSE_SHORT_HISTORY;
		}
		else
		{
			return EOperationType::OPT_CLOSE_SHORT_TODAY;
		}
	}
	else
	{
		if (off == AT_CHAR_Open)
		{
			return EOperationType::OPT_OPEN_SHORT;
		}
		else if (off == AT_CHAR_CloseYesterday)
		{
			return EOperationType::OPT_CLOSE_LONG_HISTORY;
		}
		else
		{
			return EOperationType::OPT_CLOSE_LONG_TODAY;
		}
	}
}

inline tOrderTrack* CTDEngineXt::findOrderId(int order_id)
{
	for(int i = 0; i < MMAP_ORDER_TRACK_SIZE; ++i)
	{
		if(request_track_[i].front_id == 1
				&& request_track_[i].session_id == order_id)
		{
			return request_track_ + i;
		}
	}
	return nullptr;
}

void CTDEngineXt::req_order_insert(const tIOInputOrderField* data)
{
	if((uint32_t)(data->acc_idx) < acc_utilis_.size())
	{
		auto& util = acc_utilis_[data->acc_idx];
		int off = data->off;
		int ret = 0;
		if(unlikely(ret = util->check(data->dir, off, data->price, data->vol, data->instr_hash)))
		{
			writeErrRtn(data, ret, "risk check failed");
		}
		else
		{
			auto itr = CTradeBaseInfo::instr_info_.find(data->instr_hash);
			if(itr == CTradeBaseInfo::instr_info_.end())
			{
				writeErrRtn(data, -1, "instrument not found in base info list");
				return;
			}

			AccountUnitXt& unit = account_units_[data->acc_idx];
			auto& order = unit.input_order;
			strcpy(order.m_strMarket, exchangeint2str(itr->second.exch));
			strcpy(order.m_strInstrument, data->instr);
			order.m_nVolume = data->vol;
			order.m_eOperationType = ConvJarvisOffsetToXtOffset(data->dir, off);
			order.m_dPrice = data->price;

			unit.api->order(&order, request_id_);
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
			request_track.front_id = 0; 			// flag
			request_track.session_id = 0;			// order id
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
			p->rtn_msg.order_ref = request_id_++;
			p->rtn_msg.front_id = request_track.front_id;
			p->rtn_msg.session_id = request_track.session_id;
			writer_.commit();

			util->onNew(request_track.dir, request_track.off, request_track.price, request_track.vol, request_track.instr_hash, request_track.order_ref);
		}
	}
	else
	{
		writeErrRtn(data, -1, "account idx err.");
	}
}

void CTDEngineXt::req_order_action(const tIOrderAction* data)
{
	if(unlikely(!checkRequestId(data->order_ref)))
	{
		writeErrRtn(data, -1, "cancel order reject by TDEngine: order_ref range err.", ODS(CANCEL_REJECT));
		return;
	}
	tOrderTrack& request_track = get_request_track(data->order_ref);
	if(request_track.front_id == 1)
	{
		AccountUnitXt& unit = account_units_[data->acc_idx];
		unit.api->cancel(request_track.session_id, data->order_ref);	// use session_id as order_id
		request_track.status |= ODS(CXLING);
	}
	else
	{
		writeErrRtn(data, -3, "cancel failed: xt not get OrderId yet.", ODS(CANCEL_REJECT));
	}
}

void CTDEngineXt::onOrder(int nRequestId, int orderID, const XtError& error) 
{
	LOG_DBG("onOrder nRequestId:%d orderID:%d", nRequestId, orderID);
	if(unlikely(!checkRequestId(nRequestId))) return;
	tOrderTrack& request_track = get_request_track(nRequestId);
	tIOrderRtn* p = nullptr;
	if (error.isSuccess())
	{
		request_track.status |= ODS(ACCEPT);
		request_track.front_id = 1;
		request_track.session_id = orderID;

		p = writeRtnFromTrack(request_track);
		p->rtn_msg.msg_type = ODS(ACCEPT);
		writer_.commit();

		order2request_[orderID] = nRequestId;
	}
	else
	{
		request_track.status |= ODS(REJECT);

		p = writeRtnFromTrack(request_track);
		p->rtn_msg.msg_type = ODS(REJECT);
		p->rtn_msg.errid = error.errorID();
		strncpy(p->rtn_msg.msg, error.errorMsg(), sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();
	}
	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineXt::onRtnOrderError(const COrderError* data) 
{
	LOG_DBG("onRtnOrderError nRequestId:%d orderID:%d", data->m_nRequestID, data->m_nOrderID);
	if(unlikely(!checkRequestId(data->m_nRequestID))) return;
	tOrderTrack& request_track = get_request_track(data->m_nRequestID);
	if(request_track.status & ODS(REJECT)) return;

	request_track.status |= ODS(REJECT);
	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(REJECT);
	p->rtn_msg.errid = data->m_nErrorID;
	strncpy(p->rtn_msg.msg, data->m_strErrorMsg, sizeof(p->rtn_msg.msg));
	p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineXt::onRtnOrder(const COrderInfo* data)
{
	LOG_DBG("onRtnOrder orderID:%d, status:%d", data->m_nOrderID, data->m_eStatus);
	auto itr = order2request_.find(data->m_nOrderID);
	if(unlikely(itr == order2request_.end())) return;
	tOrderTrack& request_track = get_request_track(itr->second);
	switch (data->m_eStatus)
	{
		case EOrderCommandStatus::OCS_REJECTED:
		case EOrderCommandStatus::OCS_FROCE_COMPLETED:
		case EOrderCommandStatus::OCS_CHECKFAILED:
		{
			if(request_track.status & ODS(REJECT)) return;
			request_track.status |= ODS(REJECT);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.msg_type = ODS(REJECT);
			p->rtn_msg.errid = data->m_eStatus;
			strncpy(p->rtn_msg.msg, data->m_strMsg, sizeof(p->rtn_msg.msg));
			p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
			writer_.commit();

			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
		case EOrderCommandStatus::OCS_FINISHED:
		{
			if(request_track.status & ODS(CLOSED)) return;
			request_track.status |= ODS(CLOSED);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.msg_type = ODS(CLOSED);
			p->rtn_msg.errid = data->m_eStatus;
			strncpy(p->rtn_msg.msg, data->m_strMsg, sizeof(p->rtn_msg.msg));
			p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
			writer_.commit();

			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
		case EOrderCommandStatus::OCS_STOPPED:
		{
			if(request_track.status & ODS(CANCELED)) return;
			request_track.status |= ODS(CANCELED);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.msg_type = ODS(CANCELED);
			p->rtn_msg.vol = request_track.vol - request_track.vol_traded;
			p->rtn_msg.errid = data->m_eStatus;
			strncpy(p->rtn_msg.msg, data->m_strMsg, sizeof(p->rtn_msg.msg));
			p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
			writer_.commit();

			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
//		case EOrderCommandStatus::OCS_CHECKING:
//		case EOrderCommandStatus::OCS_APPROVING:
//		case EOrderCommandStatus::OCS_RUNNING:
//		case EOrderCommandStatus::OCS_CANCELING:
//			break;
	}
}

void CTDEngineXt::onRtnOrderDetail(const COrderDetail* data)
{
	LOG_DBG("onRtnOrderDetail orderID:%d, status:%d", data->m_nOrderID, data->m_eOrderStatus);
	auto itr = order2request_.find(data->m_nOrderID);
	if(unlikely(itr == order2request_.end())) return;
	tOrderTrack& request_track = get_request_track(itr->second);
	switch (data->m_eOrderStatus)
	{
		case EEntrustStatus::ENTRUST_STATUS_WAIT_END:
		case EEntrustStatus::ENTRUST_STATUS_UNREPORTED:
		case EEntrustStatus::ENTRUST_STATUS_WAIT_REPORTING:
		case EEntrustStatus::ENTRUST_STATUS_REPORTED:
		case EEntrustStatus::ENTRUST_STATUS_UNKNOWN:
		case EEntrustStatus::ENTRUST_STATUS_REPORTED_CANCEL:
		case EEntrustStatus::ENTRUST_STATUS_PARTSUCC_CANCEL:
		case EEntrustStatus::ENTRUST_STATUS_PART_CANCEL:
		case EEntrustStatus::ENTRUST_STATUS_PART_SUCC:
		case EEntrustStatus::ENTRUST_STATUS_ACCEPT:
		case EEntrustStatus::ENTRUST_STATUS_CONFIRMED:
		case EEntrustStatus::ENTRUST_STATUS_DETERMINED:
		case EEntrustStatus::ENTRUST_STATUS_PREPARE_ORDER:
		{
			request_track.status |= ODS(MARKET_ACCEPT);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.errid = data->m_eOrderStatus;
			p->rtn_msg.msg_type = ODS(MARKET_ACCEPT);
			writer_.commit();
			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
		case EEntrustStatus::ENTRUST_STATUS_SUCCEEDED:
			request_track.status |= ODS(EXECUTION) | ODS(CLOSED);
			break;
		case EEntrustStatus::ENTRUST_STATUS_PREPARE_CANCELED:
		case EEntrustStatus::ENTRUST_STATUS_CANCELED:
		{
			if(request_track.status & ODS(CANCELED)) return;
			request_track.status |= ODS(CANCELED);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.errid = data->m_eOrderStatus;
			p->rtn_msg.msg_type = ODS(CANCELED);
			p->rtn_msg.vol = request_track.vol - request_track.vol_traded;
			strncpy(p->rtn_msg.msg, data->m_strErrorMsg, sizeof(p->rtn_msg.msg));
			p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
			writer_.commit();
			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
		case EEntrustStatus::ENTRUST_STATUS_JUNK:
		{
			request_track.status |= ODS(MARKET_REJECT);
			auto p = writeRtnFromTrack(request_track);
			p->rtn_msg.errid = data->m_eOrderStatus;
			p->rtn_msg.msg_type = ODS(MARKET_REJECT);
			writer_.commit();
			engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
			break;
		}
	}
}

void CTDEngineXt::onRtnDealDetail(const CDealDetail* data) 
{
	LOG_DBG("onRtnDealDetail orderID:%d", data->m_nOrderID);
	auto itr = order2request_.find(data->m_nOrderID);
	if(unlikely(itr == order2request_.end())) return;
	long long trade_id = strtoll(data->m_strTradeID, nullptr, 0);
	if(unlikely(testOtId(itr->second, trade_id))) return;
	tOrderTrack& request_track = get_request_track(itr->second);

	request_track.status |= ODS(EXECUTION);
	request_track.vol_traded += data->m_nVolume;
	request_track.amount_traded += data->m_dAveragePrice * data->m_nVolume;
	if(request_track.vol_traded >= request_track.vol) request_track.status |= ODS(CLOSED);

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(EXECUTION);
	p->rtn_msg.price = data->m_dAveragePrice ;
	p->rtn_msg.vol = data->m_nVolume;
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineXt::onCancel(int nRequestId, const XtError& error) 
{
	LOG_DBG("onCancel request_id:%d", nRequestId);
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		tOrderTrack& request_track = get_request_track(nRequestId);
		request_track.status |= ODS(CANCEL_REJECT);

		auto p = writeRtnFromTrack(request_track);
		p->rtn_msg.msg_type = ODS(CANCEL_REJECT);
		p->rtn_msg.errid = error.errorID();
		strncpy(p->rtn_msg.msg, error.errorMsg(), sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

void CTDEngineXt::onRtnCancelError(const CCancelError* data)
{
	LOG_DBG("onRtnCancelError request_id:%d", data->m_nRequestID);

	if(unlikely(!checkRequestId(data->m_nRequestID))) return;
	tOrderTrack& request_track = get_request_track(data->m_nRequestID);
	if (request_track.from != 0)
	{
		request_track.status |= ODS(CANCEL_REJECT);

		auto p = writeRtnFromTrack(request_track);
		p->rtn_msg.msg_type = ODS(CANCEL_REJECT);
		p->rtn_msg.errid = data->m_nErrorID;
		strncpy(p->rtn_msg.msg, data->m_strErrorMsg, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
	}
}

bool CTDEngineXt::getBaseInfo()
{
	AccountUnitXt& unit = account_units_[0];
	query_complete_flag_ = false;
	unit.api->reqAccountDetail(unit.account_id.c_str(), request_id_++);
	long start_time = CTimer::instance().getNano();
	while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timout_ns_)
	{usleep(50000);}
	if(!query_complete_flag_)
	{
		ALERT("[request] xt req account timeout");
		return false;
	}
	return queryInstruments(timout_ns_);
}

void CTDEngineXt::onReqAccountDetail(const char* accountID, int nRequestId, const CAccountDetail* data, bool isLast, const XtError& error)
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("xt req account fail %s", CEncodeConv::gbk2utf8(string(error.errorMsg())).c_str());
	}
	else
	{
		ENGLOG("trading_date=%s", data->m_strTradingDate);
		CTradeBaseInfo::trading_day_ = data->m_strTradingDate;
		query_complete_flag_ = true;
	}
}

bool CTDEngineXt::queryInstruments(long timeout_nsec)
{
	vector<string> exchs = config_["/XTTD/exchange"_json_pointer];
	for(auto &e : exchs)
	{
		query_complete_flag_ = false;
		account_units_[0].api->reqPriceDataByMarket(e.c_str(), request_id_++);
		long start_time = CTimer::instance().getNano();
		while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timeout_nsec)
		{usleep(50000);}
		if(!query_complete_flag_)
		{
			ALERT("[request] exch:%s ReqQryInstrument timeout", e.c_str());
			return false;
		}
	}
	CTradeBaseInfo::is_init_ = true;
	return true;
}

void CTDEngineXt::onReqCInstrumentDetail(const char* accountID, int nRequestId, const CInstrumentDetail* data, bool isLast, const XtError& error)
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("[request] ReqQryInstrument err:%d, %s", error.errorID(), CEncodeConv::gbk2utf8(string(error.errorMsg())).c_str());
	}
	else
	{
		if (! isLast)
		{
			uint32_t hash = INSTR_NAME_TO_HASH(data->m_strInstrumentID);
			tInstrumentInfo &info = CTradeBaseInfo::instr_info_[hash];
			info.instr_hash = hash;
			memcpy(info.instr, data->m_strInstrumentID, sizeof(info.instr));
			info.exch = exchangeStr2int(data->m_strExchangeID);
			memcpy(info.product, data->m_strProductID, sizeof(info.product));
			info.product_hash = INSTR_NAME_TO_HASH(info.product);
			info.vol_multiple = getMultipleByPrd(info.product);
			info.tick_price = data->m_dPriceTick;
			memcpy(info.expire_date, data->m_strEndDelivDate, sizeof(info.expire_date));
			info.is_trading = data->m_nSuspendedType == EXtSuspendedType::XT_NO_SUSPENDED;
		}
		else
		{
			query_complete_flag_ = true;
		}
	}
}

bool CTDEngineXt::updateOrderTrack()
{
	usleep(1000000);	// ctp limit request frequency, so sleep 1s here.
	if(CTradeBaseInfo::trading_day_ != otmmap_.getBuf()->trading_day) otmmap_.clearTrack();
	strcpy(otmmap_.getBuf()->trading_day, CTradeBaseInfo::trading_day_.c_str());
    for (int idx = 0; idx < account_units_.size(); idx ++)
    {
    	query_complete_flag_ = false;
    	curAccountIdx_ = idx;
        AccountUnitXt& unit = account_units_[idx];

		unit.api->reqOrderDetail(unit.account_id.c_str(), request_id_++);
		long start_time = CTimer::instance().getNano();
		while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timout_ns_)
		{usleep(50000);}
		if(!query_complete_flag_) return false;
    }
    request_id_ = request_id_start_ + order2request_.size();

	if(!updateTradedAmount())
	{
		ALERT("updateTradedAmount failed.");
		return false;
	}
    return true;
}

void CTDEngineXt::onReqOrderDetail(const char* accountID, int nRequestId, const COrderDetail* data, bool isLast, const XtError& error) 
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("xt req order failed! %s", CEncodeConv::gbk2utf8(string(error.errorMsg())).c_str());
	}
	else
	{
		if(isLast) query_complete_flag_ = true;
		if(data)
		{
			auto p_track = findOrderId(data->m_nOrderID);
			if(not p_track)
			{
				LOG_WARN("unknown OrderID:%d", data->m_nOrderID);
				return;
			}
			LOG_DBG("get order :%d %s", data->m_nOrderID, data->m_strInstrumentID);
			order2request_[data->m_nOrderID] = p_track - request_track_;
			p_track->status |= ODS(TDSEND);
			switch (data->m_eOrderStatus)
			{
			case EEntrustStatus::ENTRUST_STATUS_WAIT_END:
			case EEntrustStatus::ENTRUST_STATUS_UNREPORTED:
			case EEntrustStatus::ENTRUST_STATUS_WAIT_REPORTING:
			case EEntrustStatus::ENTRUST_STATUS_REPORTED:
			case EEntrustStatus::ENTRUST_STATUS_UNKNOWN:
			case EEntrustStatus::ENTRUST_STATUS_ACCEPT:
			case EEntrustStatus::ENTRUST_STATUS_CONFIRMED:
			case EEntrustStatus::ENTRUST_STATUS_DETERMINED:
			case EEntrustStatus::ENTRUST_STATUS_PREPARE_ORDER:
				p_track->status |= ODS(ACCEPT) | ODS(MARKET_ACCEPT);
				break;
			case EEntrustStatus::ENTRUST_STATUS_REPORTED_CANCEL:
			case EEntrustStatus::ENTRUST_STATUS_PARTSUCC_CANCEL:
			case EEntrustStatus::ENTRUST_STATUS_PART_CANCEL:
				p_track->status |= ODS(ACCEPT) | ODS(MARKET_ACCEPT) | ODS(CXLING);
				break;
			case EEntrustStatus::ENTRUST_STATUS_SUCCEEDED:
				p_track->status |= ODS(CLOSED);
				/* no break */
			case EEntrustStatus::ENTRUST_STATUS_PART_SUCC:
				p_track->status |= ODS(ACCEPT) | ODS(MARKET_ACCEPT)| ODS(EXECUTION);
				break;
			case EEntrustStatus::ENTRUST_STATUS_CANCELED:
			case EEntrustStatus::ENTRUST_STATUS_PREPARE_CANCELED:
				p_track->status |= ODS(CANCELED);
				break;
			case EEntrustStatus::ENTRUST_STATUS_JUNK:
				p_track->status |= ODS(MARKET_REJECT);
				break;
			default:
				break;
			}
			memcpy(p_track->instr, data->m_strInstrumentID, sizeof(p_track->instr));
			p_track->instr_hash = INSTR_NAME_TO_HASH(p_track->instr);
			p_track->price = data->m_dLimitPrice;
			p_track->vol = data->m_nTotalVolume;
			p_track->dir = data->m_nDirection == ENTRUST_BUY ? AT_CHAR_Buy : AT_CHAR_Sell;
			// track.off = pOrder->CombOffsetFlag[0];
			p_track->vol_traded = data->m_nTradedVolume;
			p_track->acc_id = curAccountIdx_;
			p_track->front_id = 1;
			p_track->session_id = data->m_nOrderID;
		}
	}
}

bool CTDEngineXt::updateTradedAmount()
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
        AccountUnitXt& unit = account_units_[idx];
		unit.api->reqDealDetail(unit.account_id.c_str(), request_id_);
		long start_time = CTimer::instance().getNano();
		while (!query_complete_flag_ && CTimer::instance().getNano() - start_time < timout_ns_)
		{usleep(50000);}
		if(!query_complete_flag_) return false;
    }
    return true;
}

void CTDEngineXt::onReqDealDetail(const char* accountID, int nRequestId, const CDealDetail* data, bool isLast, const XtError& error) 
{
	if(unlikely(!checkRequestId(nRequestId))) return;
	if (! error.isSuccess())
	{
		ALERT("xt req trade failed! %s", CEncodeConv::gbk2utf8(string(error.errorMsg())).c_str());
	}
	else
	{
		if(isLast) query_complete_flag_ = true;
		if (data)
		{
			auto p_track = findOrderId(data->m_nOrderID);
			if(p_track)
			{
				p_track->amount_traded += data->m_dAveragePrice * data->m_nVolume;
			}
		}
	}
}

int CTDEngineXt::getMultipleByPrd(const char* prd)
{
	if (strcasecmp(prd, "ni") == 0 || strcasecmp(prd, "sn") == 0 || strcasecmp(prd, "niefp") == 0 || strcasecmp(prd, "snefp") == 0)
		return 1;
	if (strcasecmp(prd, "CY") == 0 || strcasecmp(prd, "SF") == 0 || strcasecmp(prd, "SM") == 0
	 || strcasecmp(prd, "TA") == 0 || strcasecmp(prd, "al") == 0 || strcasecmp(prd, "cu") == 0
	 || strcasecmp(prd, "l") == 0 || strcasecmp(prd, "pb") == 0 || strcasecmp(prd, "pp") == 0
	 || strcasecmp(prd, "v") == 0 || strcasecmp(prd, "zn") == 0 || strcasecmp(prd, "CF") == 0
	 || strcasecmp(prd, "znefp") == 0 || strcasecmp(prd, "alefp") == 0 || strcasecmp(prd, "cuefp") == 0 || strcasecmp(prd, "pbefp") == 0)
		return 5;
	if (strcasecmp(prd, "AP") == 0 || strcasecmp(prd, "MA") == 0 || strcasecmp(prd, "b") == 0
	 || strcasecmp(prd, "bu") == 0 || strcasecmp(prd, "cs") == 0 || strcasecmp(prd, "hc") == 0
	 || strcasecmp(prd, "jd") == 0 || strcasecmp(prd, "p") == 0 || strcasecmp(prd, "rb") == 0
	 || strcasecmp(prd, "ru") == 0 || strcasecmp(prd, "wr") == 0 || strcasecmp(prd, "OI") == 0
	 || strcasecmp(prd, "RM") == 0 || strcasecmp(prd, "RS") == 0 || strcasecmp(prd, "SR") == 0
	 || strcasecmp(prd, "a") == 0 || strcasecmp(prd, "c") == 0 || strcasecmp(prd, "m") == 0
	 || strcasecmp(prd, "y") == 0 || strcasecmp(prd, "SRC") == 0 || strcasecmp(prd, "SRP") == 0
	 || strcasecmp(prd, "m_o") == 0 || strcasecmp(prd, "rbefp") == 0 || strcasecmp(prd, "ruefp") == 0
	 || strcasecmp(prd, "wrefp") == 0 || strcasecmp(prd, "buefp") == 0 || strcasecmp(prd, "fuefp") == 0
	 || strcasecmp(prd, "hcefp") == 0)
		return 10;
	if (strcasecmp(prd, "ag") == 0 || strcasecmp(prd, "agefp") == 0)
		return 15;
	if (strcasecmp(prd, "FG") == 0 || strcasecmp(prd, "JR") == 0 || strcasecmp(prd, "LR") == 0
	 || strcasecmp(prd, "RI") == 0 || strcasecmp(prd, "WH") == 0)
		return 20;
	if (strcasecmp(prd, "fu") == 0 || strcasecmp(prd, "PM") == 0)
		return 50;
	if (strcasecmp(prd, "jm") == 0)
		return 60;
	if (strcasecmp(prd, "ZC") == 0 || strcasecmp(prd, "i") == 0 || strcasecmp(prd, "j") == 0)
		return 100;
	if (strcasecmp(prd, "IC") == 0)
		return 200;
	if (strcasecmp(prd, "IH") == 0 || strcasecmp(prd, "IF") == 0)
		return 300;
	if (strcasecmp(prd, "bb") == 0 || strcasecmp(prd, "fb") == 0)
		return 500;
	if (strcasecmp(prd, "au") == 0 || strcasecmp(prd, "sc") == 0 || strcasecmp(prd, "auefp") == 0 || strcasecmp(prd, "scefp") == 0)
		return 1000;
	if (strcasecmp(prd, "TF") == 0 || strcasecmp(prd, "T") == 0 || strcasecmp(prd, "TS") == 0)
		return 10000;
	//TODO: fix this
	return 1;
}

CTDEngineXt* createTDEngineXt()
{
	return new CTDEngineXt;
}
