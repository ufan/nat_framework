/*
 * CTDEngineEES.cpp
 *
 *  Created on: Jul 20, 2018
 *      Author: hongxu
 */

#include <unistd.h>
#include "CTDEngineEES.h"
#include "CEncodeConv.h"
#include "MurmurHash2.h"
#include "utils.h"


CTDEngineEES::CTDEngineEES()
{

}

CTDEngineEES::~CTDEngineEES()
{

}

bool CTDEngineEES::init(const json& j_conf)
{
	config_ = j_conf;

	max_ref_id_ = request_id_start_ - 1;

	connect();
	if(!is_connected())
	{
		ALERT("connect to ees timeout.");
		return false;
	}

	login();
	if(!is_logged_in())
	{
		ALERT("login ees timeout.");
		return false;
	}

	return true;
}

void CTDEngineEES::connect()
{
	int timeout = config_["/EESTD/timeout"_json_pointer];

    string query_ip = config_["/EESTD/query_ip"_json_pointer];
    string trade_ip = config_["/EESTD/trade_ip"_json_pointer];
    string local_trade_ip = config_["/EESTD/local_trade_ip"_json_pointer];

	auto &j_acc = config_["/EESTD/Account"_json_pointer];
	account_units_.resize(j_acc.size());

	EES_TradeSvrInfo conn_info;
	strcpy(conn_info.m_remoteTradeIp, trade_ip.c_str());
	conn_info.m_remoteTradeTCPPort = config_["/EESTD/trade_port"_json_pointer];
	conn_info.m_remoteTradeUDPPort = config_["/EESTD/trade_port_udp"_json_pointer];
	strcpy(conn_info.m_remoteQueryIp, query_ip.c_str());
	conn_info.m_remoteQueryTCPPort = config_["/EESTD/query_port"_json_pointer];
	strcpy(conn_info.m_LocalTradeIp, local_trade_ip.c_str());

    for (int idx = 0; idx < account_units_.size(); idx ++)
    {
        auto& unit = account_units_[idx];
        if (unit.api == nullptr)
        {
        	unit.api = CreateEESTraderApi();
            if (!unit.api)
            {
                throw std::runtime_error("EES_TD failed to create api");
            }
        }
        if (!unit.connected)
        {
            curAccountIdx_ = idx;
            conn_info.m_LocalTradeUDPPort = j_acc[idx]["local_udp_port"];
            RESULT res = unit.api->ConnServer(conn_info, this);
        	if (res != NO_ERROR)
        	{
        		ALERT("connect to EES server failed!\n");
        		return;
        	}

            time_t start_time = time(NULL);
            while (!unit.connected && time(NULL) - start_time < timeout)
            {usleep(50000);}
        }
    }
}

bool CTDEngineEES::is_connected() const
{
    for (int i = 0; i < account_units_.size(); ++i)
    {
    	auto &unit = account_units_[i];
        if (!unit.connected)
        {
        	LOG_INFO("account idx:%d not connected.", i);
            return false;
        }
    }
    return true;
}

void CTDEngineEES::login()
{
	int timeout = config_["/EESTD/timeout"_json_pointer];
	auto &j_acc = config_["/EESTD/Account"_json_pointer];
    for (int idx = 0; idx < account_units_.size(); idx ++)
    {
        auto &unit = account_units_[idx];
        // login
        if (!unit.logged_in)
        {
            curAccountIdx_ = idx;
            string login_id = j_acc[idx]["login_id"];
            string passwd = j_acc[idx]["passwd"];
            string nic_mac = j_acc[idx]["nic_mac"];

            RESULT res = unit.api->UserLogon(login_id.c_str(), passwd.c_str(), "jarvis", nic_mac.c_str());
            if (res != NO_ERROR)
            {
                ALERT("login failed! id:%s", login_id.c_str());
                return;
            }
            time_t start_time = time(NULL);
            while (!unit.logged_in && time(NULL) - start_time < timeout)
            {usleep(50000);}
            if(!unit.logged_in)
            {
                ALERT("login timeout! id:%s", login_id.c_str());
                return;
            }

            unit.logged_in = false;
            res = unit.api->QueryUserAccount();
            if (res != NO_ERROR)
            {
                ALERT("QueryUserAccount failed! id:%s", login_id.c_str());
                return;
            }
            start_time = time(NULL);
            while (!unit.logged_in && time(NULL) - start_time < timeout)
            {usleep(50000);}
            if(!unit.logged_in)
            {
                ALERT("QueryUserAccount timeout! id:%s", login_id.c_str());
                return;
            }
        }

    }
}

bool CTDEngineEES::is_logged_in() const
{
    for (int i = 0; i < account_units_.size(); ++i)
    {
    	auto &unit = account_units_[i];
        if (!unit.logged_in)
        {
        	LOG_INFO("account idx:%d not login.", i);
            return false;
        }
    }
    return true;
}

void CTDEngineEES::release()
{
    for (auto &unit: account_units_)
    {
        if(unit.connected)
        {
        	unit.api->DisConnServer();
        	unit.connected = false;
        	unit.logged_in = false;
        }
        if(unit.api)
        {
        	DestroyEESTraderApi(unit.api);
        	unit.api = nullptr;
        }
    }
}

bool CTDEngineEES::queryInstruments()
{
	int timeout = config_["/EESTD/timeout"_json_pointer];

	RESULT res = account_units_[0].api->QuerySymbolList();
	if(NO_ERROR != res)
	{
		ALERT("[request] QuerySymbolList err:%d, ", res);
		return false;
	}

    time_t start_time = time(NULL);
    while (!CTradeBaseInfo::is_init_ && time(NULL) - start_time < timeout)
    {usleep(50000);}

    if(!CTradeBaseInfo::is_init_)
    {
    	ALERT("[request] QuerySymbolList timeout");
    	return false;
    }
	return true;
}

bool CTDEngineEES::updateOrderTrack()
{
	if(CTradeBaseInfo::trading_day_ != otmmap_.getBuf()->trading_day) otmmap_.clearTrack();
	strcpy(otmmap_.getBuf()->trading_day, CTradeBaseInfo::trading_day_.c_str());

	int timeout = config_["/EESTD/timeout"_json_pointer];
	auto &j_acc = config_["/EESTD/Account"_json_pointer];

	for (int idx = 0; idx < account_units_.size(); idx ++)
    {
    	query_complete_flag_ = false;
    	curAccountIdx_ = idx;
        auto& unit = account_units_[idx];

        string login_id = j_acc[idx]["login_id"];
        RESULT ret_err = unit.api->QueryAccountOrder(login_id.c_str());
        if (ret_err != NO_ERROR)
		{
			ALERT("QueryAccountOrder failed: %d", ret_err);
			return  false;
		}
        time_t start_time = time(NULL);
        while (!query_complete_flag_ && time(NULL) - start_time < timeout)
        {usleep(50000);}
		if(!query_complete_flag_) return false;
    }

	request_id_ = (max_ref_id_ + 1 > request_id_) ? max_ref_id_ + 1 : request_id_;
	LOG_DBG("request_id: %d", request_id_);

	if(!updateTradedAmount())
	{
		ALERT("updateTradedAmount failed.");
		return false;
	}
    return true;
}

bool CTDEngineEES::updateTradedAmount()
{
	for(int i = 0; i < MMAP_ORDER_TRACK_SIZE; i++)
	{
		 tOrderTrack& ot = get_request_track(i);
		 ot.amount_traded = 0.0;
	}

	int timeout = config_["/EESTD/timeout"_json_pointer];
	auto &j_acc = config_["/EESTD/Account"_json_pointer];

    for (int idx = 0; idx < account_units_.size(); idx ++)
    {
    	query_complete_flag_ = false;
    	curAccountIdx_ = idx;
        auto& unit = account_units_[idx];

        string login_id = j_acc[idx]["login_id"];
        RESULT ret_err = unit.api->QueryAccountOrderExecution(login_id.c_str());
        if (ret_err != NO_ERROR)
		{
			ALERT("QueryAccountOrderExecution failed: %d", ret_err);
			return  false;
		}
        time_t start_time = time(NULL);
        while (!query_complete_flag_ && time(NULL) - start_time < timeout)
        {usleep(50000);}
		if(!query_complete_flag_) return false;
    }
    return true;
}

void CTDEngineEES::req_order_insert(const tIOInputOrderField* data)
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
		auto& unit = account_units_[data->acc_idx];
		auto& order = unit.order;
		if(data->dir == AT_CHAR_Buy)
		{
			switch(data->off)
			{
			case AT_CHAR_Open: order.m_Side = EES_SideType_open_long; break;
			case AT_CHAR_CloseToday: order.m_Side = EES_SideType_close_today_short; break;
			case AT_CHAR_CloseYesterday:
			case AT_CHAR_Close: order.m_Side = EES_SideType_close_ovn_short; break;
			default: ;
			}
		}
		else
		{
			switch(data->off)
			{
			case AT_CHAR_Open: order.m_Side = EES_SideType_open_short; break;
			case AT_CHAR_CloseToday: order.m_Side = EES_SideType_close_today_long; break;
			case AT_CHAR_CloseYesterday:
			case AT_CHAR_Close: order.m_Side = EES_SideType_close_ovn_long; break;
			default: ;
			}
		}
		strncpy(order.m_Symbol, data->instr, sizeof(order.m_Symbol));
		order.m_Symbol[sizeof(order.m_Symbol) - 1] = '\0';

		auto itr = CTradeBaseInfo::instr_info_.find(data->instr_hash);
		if(itr == CTradeBaseInfo::instr_info_.end())
		{
			writeErrRtn(data, -1, "instrument not found in base info list");
			return;
		}
		order.m_Exchange = itr->second.exch;
		order.m_Price = data->price;
		order.m_Qty = data->vol;
		order.m_ClientOrderToken = request_id_;

		RESULT ret_err = unit.api->EnterOrder(&order);
		if (ret_err == NO_ERROR) // succ, then request_id_ should ++
		{
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
			request_track.front_id = 0;			// this field is to be filled with MarketOrderToken
			request_track.session_id = 0;		// indicate that MarketOrderToken not get yet
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

			util->onNew(data->dir, off, data->price, data->vol, data->instr_hash, request_track.order_ref);
		}
		else
		{
			writeErrRtn(data, ret_err, "EES EnterOrder failed.");
		}
	}
}

void CTDEngineEES::req_order_action(const tIOrderAction* data)
{
	if(unlikely(!checkRequestId(data->order_ref)))
	{
		writeErrRtn(data, -1, "cancel order reject by TDEngine: order_ref range err.", ODS(CANCEL_REJECT));
		return;
	}
	tOrderTrack& request_track = get_request_track(data->order_ref);
	if(request_track.session_id == 0)
	{
		writeErrRtn(data, -3, "cancel failed: ees not get MarketToken yet.", ODS(CANCEL_REJECT));
		return;
	}

	auto& unit = account_units_[data->acc_idx];
	auto& order = unit.action_order;
	order.m_MarketOrderToken = data->front_id;

	RESULT ret_err = unit.api->CancelOrder(&order);
	if(ret_err == NO_ERROR)
	{
		request_track.status |= ODS(CXLING);
	}
	else
	{
		writeErrRtn(data, -3, "ees CancelOrder api failed.", ODS(CANCEL_REJECT));
	}
}

void CTDEngineEES::OnConnection(ERR_NO errNo, const char* pErrStr)
{
	if (errNo != NO_ERROR)
	{
        string msg = CEncodeConv::gbk2utf8(pErrStr);
		LOG_ERR("connect to rem server failed(%d), %s!\n", errNo, msg.c_str());
		return;
	}
	account_units_[curAccountIdx_].connected = true;
}

void CTDEngineEES::OnDisConnection(ERR_NO errNo, const char* pErrStr)
{
    string msg = CEncodeConv::gbk2utf8(pErrStr);
	ALERT("[OnFrontDisconnected] err_no=%d, msg:%s", errNo, msg.c_str());
    for (auto& unit: account_units_)
    {
        unit.connected = false;
        unit.logged_in = false;
    }
    stop();
}

void CTDEngineEES::OnUserLogon(EES_LogonResponse* pLogon)
{
	if (pLogon->m_Result != NO_ERROR)
	{
		ALERT("logon failed, result=%d\n", pLogon->m_Result);
		return;
	}
	account_units_[curAccountIdx_].logged_in = true;

	char buf[50];
	int2string(buf, pLogon->m_TradingDate);
	CTradeBaseInfo::trading_day_ = buf;		// update trading day here.

	max_ref_id_ = max_ref_id_ > pLogon->m_MaxToken ? max_ref_id_ : pLogon->m_MaxToken;
	request_id_ = (max_ref_id_ + 1 > request_id_) ? max_ref_id_ + 1 : request_id_;

	LOG_TRACE("idx:%d logon successfully, trading date(%u), max token(%d)\n", curAccountIdx_, pLogon->m_TradingDate, pLogon->m_MaxToken);
}

void CTDEngineEES::OnQueryUserAccount(EES_AccountInfo* pAccoutnInfo, bool bFinish)
{
    auto &unit = account_units_[curAccountIdx_];
    if(bFinish)
    {
        unit.logged_in = true;
    }
    else
    {
        strcpy(unit.order.m_Account, pAccoutnInfo->m_Account);
        strcpy(unit.action_order.m_Account, pAccoutnInfo->m_Account);
        unit.action_order.m_Quantity = 0;
    }
}

void CTDEngineEES::OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish)
{
    if(bFinish)
    {
    	CTradeBaseInfo::is_init_ = true;
    }
    else
    {
		uint32_t hash = INSTR_NAME_TO_HASH(pSymbol->m_symbol);
		tInstrumentInfo &info = CTradeBaseInfo::instr_info_[hash];
		memset(&info, 0, sizeof(info));
		info.instr_hash = hash;

		strncpy(info.instr, pSymbol->m_symbol, sizeof(info.instr)-1);
		info.exch = pSymbol->m_ExchangeID;
		strncpy(info.product, pSymbol->m_ProdID, sizeof(info.product)-1);
		info.product_hash = INSTR_NAME_TO_HASH(info.product);
		info.vol_multiple = pSymbol->m_VolumeMultiple;
		info.tick_price = pSymbol->m_PriceTick;

		char buf[50];
		int2string(buf, pSymbol->m_ExpireDate);
		strncpy(info.expire_date, buf, sizeof(info.expire_date)-1);
		info.is_trading = pSymbol->m_IsTrading;
    }
}

void CTDEngineEES::OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish)
{
	if(bFinish) query_complete_flag_ = true;
	else
	{
		long order_ref = pQueryOrder->m_ClientOrderToken;
		if(unlikely(!checkRequestId(order_ref))) return;
		tOrderTrack& track = get_request_track(order_ref);

		strncpy(track.instr, pQueryOrder->m_symbol, sizeof(track.instr)-1);
		track.instr[sizeof(track.instr)-1] = '\0';
		track.instr_hash = INSTR_NAME_TO_HASH(track.instr);

		track.price = pQueryOrder->m_Price;
		track.vol = pQueryOrder->m_Quantity;

		switch(pQueryOrder->m_SideType)
		{
	    case EES_SideType_open_long:
			track.dir = AT_CHAR_Buy;
			track.off = AT_CHAR_Open;
	    	break;
	    case EES_SideType_close_today_long:
			track.dir = AT_CHAR_Sell;
			track.off = AT_CHAR_CloseToday;
	    	break;
	    case EES_SideType_close_today_short:
			track.dir = AT_CHAR_Buy;
			track.off = AT_CHAR_CloseToday;
	    	break;
	    case EES_SideType_open_short:
			track.dir = AT_CHAR_Sell;
			track.off = AT_CHAR_Open;
	    	break;
	    case EES_SideType_close_ovn_short:
			track.dir = AT_CHAR_Buy;
			track.off = AT_CHAR_CloseYesterday;
	    	break;
	    case EES_SideType_close_ovn_long:
			track.dir = AT_CHAR_Sell;
			track.off = AT_CHAR_CloseYesterday;
	    	break;
	    default:
	    	LOG_ERR("unknown order side %d", pQueryOrder->m_SideType);
		}

		track.vol_traded = pQueryOrder->m_FilledQty;
		track.acc_id = curAccountIdx_;
		track.order_ref = order_ref;
		track.front_id = pQueryOrder->m_MarketOrderToken;
		track.session_id = 1;

		track.status = ODS(TDSEND);
		if(EES_OrderStatus_cxl_requested & pQueryOrder->m_OrderStatus) track.status |= ODS(CXLING);
		if(EES_OrderStatus_cancelled & pQueryOrder->m_OrderStatus) track.status |= ODS(CANCELED);
		if(EES_OrderStatus_executed & pQueryOrder->m_OrderStatus) track.status |= ODS(EXECUTION);
		if(EES_OrderStatus_mkt_accept & pQueryOrder->m_OrderStatus) track.status |= ODS(MARKET_ACCEPT);
		if(EES_OrderStatus_shengli_accept & pQueryOrder->m_OrderStatus) track.status |= ODS(ACCEPT);
		if(EES_OrderStatus_closed & pQueryOrder->m_OrderStatus)
		{
			if(!(EES_OrderStatus_shengli_accept & pQueryOrder->m_OrderStatus)) track.status |= ODS(REJECT);
			else if(!(EES_OrderStatus_mkt_accept & pQueryOrder->m_OrderStatus)) track.status |= ODS(MARKET_REJECT);
		}
		if(track.vol_traded >= track.vol) track.status |= ODS(CLOSED);

		max_ref_id_ = (order_ref > max_ref_id_) ? order_ref : max_ref_id_;
	}
	return;
}

void CTDEngineEES::OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish)
{
	if(bFinish) query_complete_flag_ = true;
	else
	{
		long order_ref = pQueryOrderExec->m_ClientOrderToken;
		if(unlikely(!checkRequestId(order_ref))) return;
		tOrderTrack& track = get_request_track(order_ref);
		track.amount_traded += pQueryOrderExec->m_ExecutionPrice * pQueryOrderExec->m_ExecutedQuantity;
	}
}

void CTDEngineEES::OnOrderAccept(EES_OrderAcceptField* pAccept)
{
	if(unlikely(!checkRequestId(pAccept->m_ClientOrderToken))) return;
	tOrderTrack& request_track = get_request_track(pAccept->m_ClientOrderToken);
	if(request_track.status & ODS(ACCEPT)) return;

	request_track.status |= ODS(ACCEPT);
	request_track.front_id = pAccept->m_MarketOrderToken;
	request_track.session_id = 1;

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(ACCEPT);
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnOrderReject(EES_OrderRejectField* pReject)
{
	if(unlikely(!checkRequestId(pReject->m_ClientOrderToken))) return;
	tOrderTrack& request_track = get_request_track(pReject->m_ClientOrderToken);
	if(request_track.status & ODS(REJECT)) return;

	request_track.status |= ODS(REJECT);

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(REJECT);
	p->rtn_msg.errid = pReject->m_ReasonCode;
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept)
{
	if(unlikely(!checkRequestId(pAccept->m_ClientOrderToken))) return;
	tOrderTrack& request_track = get_request_track(pAccept->m_ClientOrderToken);
	if(request_track.status & ODS(MARKET_ACCEPT)) return;

	request_track.status |= ODS(MARKET_ACCEPT);
	request_track.front_id = pAccept->m_MarketOrderToken;
	request_track.session_id = 1;

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(MARKET_ACCEPT);
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnOrderMarketReject(EES_OrderMarketRejectField* pReject)
{
	if(unlikely(!checkRequestId(pReject->m_ClientOrderToken))) return;
	tOrderTrack& request_track = get_request_track(pReject->m_ClientOrderToken);
	if(request_track.status & ODS(MARKET_REJECT)) return;

	request_track.status |= ODS(MARKET_REJECT);
	request_track.front_id = pReject->m_MarketOrderToken;
	request_track.session_id = 1;

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(MARKET_REJECT);
	p->rtn_msg.errid = -1;
	strncpy(p->rtn_msg.msg, pReject->m_ReasonText, sizeof(p->rtn_msg.msg));
	p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnOrderExecution(EES_OrderExecutionField* pExec)
{
	if(unlikely(!checkRequestId(pExec->m_ClientOrderToken))) return;
	if(unlikely(testOtId(pExec->m_ClientOrderToken, pExec->m_ExecutionID))) return;
	tOrderTrack& request_track = get_request_track(pExec->m_ClientOrderToken);
	request_track.status |= ODS(EXECUTION);
	request_track.vol_traded += pExec->m_Quantity;
	request_track.amount_traded += pExec->m_Price * pExec->m_Quantity;
	request_track.front_id = pExec->m_MarketOrderToken;
	request_track.session_id = 1;
	if(request_track.vol_traded >= request_track.vol) request_track.status |= ODS(CLOSED);

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(EXECUTION);
	p->rtn_msg.price = pExec->m_Price ;
	p->rtn_msg.vol = pExec->m_Quantity;
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnOrderCxled(EES_OrderCxled* pCxled)
{
	if(unlikely(!checkRequestId(pCxled->m_ClientOrderToken))) return;
	tOrderTrack& request_track = get_request_track(pCxled->m_ClientOrderToken);
	if(request_track.status & ODS(CANCELED)) return;

	request_track.status |= ODS(CANCELED);
	request_track.front_id = pCxled->m_MarketOrderToken;
	request_track.session_id = 1;

	auto p = writeRtnFromTrack(request_track);
	p->rtn_msg.msg_type = ODS(CANCELED);
	p->rtn_msg.vol = pCxled->m_Decrement;
	p->rtn_msg.errid = pCxled->m_Reason;
	writer_.commit();

	engine_on_rtn(request_track.acc_id, &request_track, &(p->rtn_msg));
}

void CTDEngineEES::OnCxlOrderReject(EES_CxlOrderRej* pReject)
{
	tOrderTrack *p_track = nullptr;
	for(int i = request_id_ - 1; i >= request_id_start_; i--)
	{
		tOrderTrack& request_track = get_request_track(i);
		if(request_track.session_id && request_track.front_id == pReject->m_MarketOrderToken)
		{
			p_track = &request_track;
			break;
		}
	}
	if(p_track)
	{
		p_track->status |= ODS(CANCEL_REJECT);
		p_track->front_id = pReject->m_MarketOrderToken;
		p_track->session_id = 1;

		auto p = writeRtnFromTrack(*p_track);
		p->rtn_msg.msg_type = ODS(CANCEL_REJECT);
		p->rtn_msg.errid = pReject->m_ReasonCode;
		strncpy(p->rtn_msg.msg, pReject->m_ReasonText, sizeof(p->rtn_msg.msg));
		p->rtn_msg.msg[sizeof(p->rtn_msg.msg) - 1] = '\0';
		writer_.commit();

		engine_on_rtn(p_track->acc_id, p_track,  &(p->rtn_msg));
	}
	else
	{
		LOG_ERR("OnCxlOrderReject: cannot find order track match MarketToken:%lld", pReject->m_MarketOrderToken);
	}
}

