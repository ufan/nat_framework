/*
 * CCtpTrader.cpp
 *
 *  Created on: 2017年12月19日
 *      Author: hongxu
 */

#include <stdlib.h>
#include <vector>
#include "CCtpTrader.h"
#include "CConfig.h"
#include "CGlobalParameter.h"
#include "SimpleLogger.h"
#include "CEncodeConv.h"

static const char* parseOrderStatus(char flag)
{
	const char *info = NULL;
	switch(flag)
	{
	case THOST_FTDC_OST_Unknown:
		info = "AcceptByDesk";
		break;
	case THOST_FTDC_OST_Canceled:
		info = "CancelByExchange";
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
		info = "NoTradeQueueing";
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		info = "PartTradedQueueing";
		break;
	case THOST_FTDC_OST_PartTradedNotQueueing:
		info = "PartTradedNotQueueing";
		break;
	case THOST_FTDC_OST_AllTraded:
		info = "AllTraded";
		break;
	default:
		info = "Unknown status";
	}
	return info;
}

static const char* parseDir(char flag)
{
	const char *dir = "unknown";
	switch(flag)
	{
	case '0': dir = "buy"; break;
	case '1': dir = "sell"; break;
	default:;
	}
	return dir;
}

static const char* parseOffsetFlag(char flag)
{
	const char *off = "unknown";
	switch(flag)
	{
	case THOST_FTDC_OF_Open: off = "open"; break;
	case THOST_FTDC_OF_Close: off = "close"; break;
	case THOST_FTDC_OF_CloseToday: off = "close_today"; break;
	case THOST_FTDC_OF_ForceClose: off = "force_close"; break;
	case THOST_FTDC_OF_CloseYesterday: off = "close_yesterday"; break;
	case THOST_FTDC_OF_ForceOff: off = "force_off"; break;
	case THOST_FTDC_OF_LocalForceClose: off = "local_force_close"; break;
	default:;
	}
	return off;
}

static const char* parsePosDir(char flag)
{
	const char *dir = "unknown";
	switch(flag)
	{
	case '1': dir = "net"; break;
	case '2': dir = "long"; break;
	case '3': dir = "short"; break;
	default:;
	}
	return dir;
}


CCtpTrader::CCtpTrader() : p_api_(NULL), request_id_(0), order_id_(0)
{

}

CCtpTrader::~CCtpTrader()
{

}

bool CCtpTrader::init()
{
	if(p_api_ ==  NULL)
	{
		CConfig *p_cnf = CGlobalParameter::getConfig();

		broker_id_ = p_cnf->getVal<string>("CTP", "broker_id");
		user_id_ = p_cnf->getVal<string>("CTP", "user_id");

		string flow_path = p_cnf->getVal<string>("CTP", "trade_flow_path");
		CThostFtdcTraderApi *api = CThostFtdcTraderApi::CreateFtdcTraderApi(flow_path.c_str());
		p_api_ = api;

		api->RegisterSpi(this);

		vector<string> front_list;
		p_cnf->getValList("CTP", "front", front_list);
		for(auto& it: front_list) api->RegisterFront((char*)it.c_str());

		api->SubscribePrivateTopic(THOST_TERT_QUICK);
		api->SubscribePublicTopic(THOST_TERT_QUICK);
		api->Init();
		return true;
	}

	LOG_ERR("CCtpTrader is already inited");
	return false;
}

void CCtpTrader::join()
{
	p_api_->Join();
}

void CCtpTrader::OnFrontConnected()
{
	LOG_TRACE("Front Connected.");
	reqUserLogin();
}

void CCtpTrader::reqUserLogin()
{
	CConfig *p_cnf = CGlobalParameter::getConfig();

	CThostFtdcReqUserLoginField login_req;
	memset(&login_req, 0, sizeof(login_req));

	strcpy(login_req.BrokerID, broker_id_.c_str());
	strcpy(login_req.UserID, user_id_.c_str());

	string pass = p_cnf->getVal<string>("CTP", "passwd");
	strcpy(login_req.Password, pass.c_str());
	LOG_TRACE("CCtpTrader: broker_id=[%s], user_id=[%s], passwd=[%c*****]", broker_id_.c_str(), user_id_.c_str(), pass[0]);

	int ret = p_api_->ReqUserLogin(&login_req, ++request_id_);
	parseRetCode(ret, "login");
}

void CCtpTrader::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
	{
		string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
		LOG_ERR("CCtpTrader: User login fail! ErrID=[%d], ErrMsg=[%s];", pRspInfo->ErrorID, msg.c_str());
		return;
	}

	front_id_ = pRspUserLogin->FrontID;
	session_id_ = pRspUserLogin->SessionID;
	order_ref_ = pRspUserLogin->MaxOrderRef;
	order_id_ = atoi(order_ref_.c_str());

	LOG_TRACE("CCtpTrader: User login succ. FrontID=[%d], SessionID=[%d], MaxOrderRef=[%s].", front_id_, session_id_, order_ref_.c_str());

	// ctp mini don't need this.
	reqSettlementInfoConfirm();
}

void CCtpTrader::reqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField settlementConfirmReq;
	memset(&settlementConfirmReq, 0, sizeof(settlementConfirmReq));

	strcpy(settlementConfirmReq.BrokerID, broker_id_.c_str());
	strcpy(settlementConfirmReq.InvestorID, user_id_.c_str());
	int ret = p_api_->ReqSettlementInfoConfirm(&settlementConfirmReq, ++request_id_);
	parseRetCode(ret, "settlement confirm");
}

void CCtpTrader::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
	{
		string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
		LOG_ERR("CCtpTrader: SettlementInfo confirmed fail! ErrID=[%d], ErrMsg=[%s];", pRspInfo->ErrorID, msg.c_str());
	}
	else
	{
		LOG_TRACE("CCtpTrader: SettlementInfo confirmed succ.");
	}

	CGlobalParameter::terminal_lock_ = false;
}

void CCtpTrader::parseRetCode(int code, string prefix)
{
	switch(code)
	{
		case 0:
			LOG_INFO("CCtpTrader: %s request sent succ.", prefix.c_str());
			break;

		case -1:
			LOG_ERR("CCCtpTraderr: %s request sent fail. network error.", prefix.c_str());
			break;

		case -2:
			LOG_ERR("CCtpTrader: %s request sent fail. queue quantity exceed.", prefix.c_str());
			break;

		case -3:
			LOG_ERR("CCtpTrader: %s request sent fail. req intensity exceed.", prefix.c_str());
			break;
	}
}

int CCtpTrader::qryPosition(const char *inst_idstr)
{
	printf("Instrument          Dir         YdPos       TodayPos    Position    PosProfit     UseMargin\n");
	CThostFtdcQryInvestorPositionField pos;
	memset(&pos, 0, sizeof(pos));
	strcpy(pos.BrokerID,  broker_id_.c_str());
	strcpy(pos.InvestorID, user_id_.c_str());

	if (inst_idstr != NULL)
	{
        strcpy(pos.InstrumentID, inst_idstr);
	}

	int ret = p_api_->ReqQryInvestorPosition(&pos, ++request_id_);
	parseRetCode(ret, "position");
	return ret;
}

int CCtpTrader::qryAccount()
{
	printf("AccountID   Available     CloseProfit   PosProfit     CurrMargin    FrozenMargin\n");
	CThostFtdcQryTradingAccountField acc;
	memset(&acc, 0, sizeof(acc));
	strcpy(acc.BrokerID, broker_id_.c_str());
	strcpy(acc.InvestorID, user_id_.c_str());
	int ret = p_api_->ReqQryTradingAccount(&acc, ++request_id_);
	parseRetCode(ret, "account");
	return ret;
}

void CCtpTrader::OnFrontDisconnected(int nReason)
{
	LOG_ERR("CCtpTrader: Disconnected, nReason=[%d];", nReason);
}

void CCtpTrader::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pRspInfo)
	{
		const char *dir = parseDir(pInputOrder->Direction);
		const char *off = parseOffsetFlag(pInputOrder->CombOffsetFlag[0]);
		if(pRspInfo->ErrorID == 0)
		{
			LOG_TRACE("Order inserted: %s %s inst %s price %lf vol %d", dir, off, pInputOrder->InstrumentID, pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal);
		}
		else
		{
			string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
			LOG_ERR("Order insert err: %s %s inst %s price %lf vol %d info [%d]%s", dir, off, pInputOrder->InstrumentID, pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal,
					pRspInfo->ErrorID, msg.c_str());
		}
	}
}

void CCtpTrader::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
	const char *dir = parseDir(pInputOrder->Direction);
	const char *off = parseOffsetFlag(pInputOrder->CombOffsetFlag[0]);
	LOG_ERR("Err|Order %s %s inst %s price %lf vol %d, info [%d]%s", dir, off,
			pInputOrder->InstrumentID, pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal,
			pRspInfo->ErrorID, msg.c_str());
}

void CCtpTrader::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pInputOrderAction)
	{
		string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
		LOG_TRACE("Order canceled: inst %s price %lf vol %d info [%d]%s",
				pInputOrderAction->InstrumentID, pInputOrderAction->LimitPrice,
				pInputOrderAction->VolumeChange, pRspInfo->ErrorID, msg.c_str());
	}
}

void CCtpTrader::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	string msg = CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg);
	LOG_ERR("Order cancel err: inst %s price %lf vol %d info [%d]%s",
			pOrderAction->InstrumentID, pOrderAction->LimitPrice,
			pOrderAction->VolumeChange, pRspInfo->ErrorID, msg.c_str());
}

void CCtpTrader::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	const char *dir = parseDir(pOrder->Direction);
	const char *off = parseOffsetFlag(pOrder->CombOffsetFlag[0]);
	const char *info = parseOrderStatus(pOrder->OrderStatus);
	string msg = CEncodeConv::gbk2utf8(pOrder->StatusMsg);
	LOG_TRACE("OnRtnOrder: %s %s Inst %s, Price %lf, VolTotOri %d, VolTot %d, VolTraded %d, Status %s, Msg %s",
			dir, off,
			pOrder->InstrumentID, pOrder->LimitPrice,
			pOrder->VolumeTotalOriginal, pOrder->VolumeTotal,
			pOrder->VolumeTraded, info, msg.c_str());
}

void CCtpTrader::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	const char *dir = parseDir(pTrade->Direction);
	const char *off = parseOffsetFlag(pTrade->OffsetFlag);
	LOG_TRACE("OnRtnTrade: %s %s inst %s price %lf vol %d", dir, off, pTrade->InstrumentID, pTrade->Price, pTrade->Volume);
}

void CCtpTrader::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *p, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(p)
	{
		if(p->Position > 0)
		{
			const char *dir = parsePosDir(p->PosiDirection);
			printf("%-18s  %-10s  %-10d  %-10d  %-10d  %-12.2f  %-12.2f\n", p->InstrumentID, dir, p->YdPosition, p->TodayPosition, p->Position, p->PositionProfit, p->UseMargin);
		}
	}

	if(bIsLast) CGlobalParameter::terminal_lock_ = false;
}

void CCtpTrader::OnRspQryTradingAccount(CThostFtdcTradingAccountField *p, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(p)
	{
		printf("%-10s  %-12.2f  %-12.2f  %-12.2f  %-12.2f  %-12.2f\n", p->AccountID, p->Available, p->CloseProfit, p->PositionProfit, p->CurrMargin, p->FrozenMargin);
	}

	if(bIsLast) CGlobalParameter::terminal_lock_ = false;
}

int CCtpTrader::sendOrder(string instrument, double price, int volume, int direction, int offset)
{
	CThostFtdcInputOrderField order;
	memset(&order, 0, sizeof(order));

	strcpy(order.BrokerID, broker_id_.c_str());
	strcpy(order.InvestorID, user_id_.c_str());
	order.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	order.ContingentCondition = THOST_FTDC_CC_Immediately;
	order.VolumeCondition = THOST_FTDC_VC_AV;
	order.MinVolume = 1;
	order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	order.IsAutoSuspend = 0;
	order.UserForceClose = 0;
	order.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	order.TimeCondition = THOST_FTDC_TC_GFD;
	snprintf(order.OrderRef, sizeof(order.OrderRef), "%d", order_id_++);

	strcpy(order.InstrumentID, instrument.c_str());
	order.VolumeTotalOriginal = volume;
	order.CombOffsetFlag[0] = offset;
	order.Direction = direction;
	order.LimitPrice = price;

	int ret = p_api_->ReqOrderInsert(&order, ++request_id_);
	parseRetCode(ret, "order insert");
	return ret;
}

int CCtpTrader::deleteOrder(string ref)
{
	int front_id = 0, session_id = 0;
	string orderref;
	string inst;
	stringstream ss;
	ss << ref;
	ss >> front_id >> session_id >> orderref >> inst;

	CThostFtdcInputOrderActionField order;
	memset(&order, 0, sizeof(order));

	strcpy(order.OrderRef, orderref.c_str());
	strcpy(order.BrokerID, broker_id_.c_str());
	strcpy(order.InvestorID, user_id_.c_str());
	order.FrontID = front_id;
	order.SessionID = session_id;
	order.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy(order.InstrumentID, inst.c_str());

	int ret = p_api_->ReqOrderAction(&order, ++request_id_);
	parseRetCode(ret, "delete order");
	return ret;
}

int CCtpTrader::qryOrder()
{
	printf("ID    FrontID  SessID       OrderRef   Instrument         Dir   Off                Price        VolTotOri  VolTot     VolTrade   Status\n");
	id_ = 0;
	CGlobalParameter::order_id_map_.clear();

	CThostFtdcQryOrderField query;
	memset(&query, 0, sizeof(query));

	strcpy(query.BrokerID, broker_id_.c_str());
	strcpy(query.InvestorID, user_id_.c_str());

	int ret = p_api_->ReqQryOrder(&query, ++request_id_);
	parseRetCode(ret, "query order");
	return ret;
}

int CCtpTrader::qryTrade()
{
	printf("OrderRef   Instrument         Dir   Off                Price        Vol\n");
	CThostFtdcQryTradeField query;
	memset(&query, 0, sizeof(query));

	strcpy(query.BrokerID, broker_id_.c_str());
	strcpy(query.InvestorID, user_id_.c_str());

	int ret = p_api_->ReqQryTrade(&query, ++request_id_);
	parseRetCode(ret, "query order");
	return ret;
}

void CCtpTrader::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pOrder)
	{
		const char *dir = parseDir(pOrder->Direction);
		const char *off = parseOffsetFlag(pOrder->CombOffsetFlag[0]);
		const char *status = parseOrderStatus(pOrder->OrderStatus);

		printf("%-5d %-8d %-12d %-10s %-18s %-5s %-18s %-12.2f %-10d %-10d %-10d %s\n", id_, pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef, pOrder->InstrumentID, dir, off,
				pOrder->LimitPrice, pOrder->VolumeTotalOriginal,
				pOrder->VolumeTotal, pOrder->VolumeTraded, status);

		stringstream ss;
		ss << pOrder->FrontID << " " << pOrder->SessionID << " " << pOrder->OrderRef << " " << pOrder->InstrumentID;
		CGlobalParameter::order_id_map_[id_] = ss.str();
		id_++;
	}

	if(bIsLast) CGlobalParameter::terminal_lock_ = false;
}

void CCtpTrader::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pTrade)
	{
		const char *dir = parseDir(pTrade->Direction);
		const char *off = parseOffsetFlag(pTrade->OffsetFlag);

		printf("%-10s %-18s %-5s %-18s %-12.2f %-10d\n", pTrade->OrderRef, pTrade->InstrumentID, dir, off, pTrade->Price, pTrade->Volume);
	}

	if(bIsLast) CGlobalParameter::terminal_lock_ = false;
}

