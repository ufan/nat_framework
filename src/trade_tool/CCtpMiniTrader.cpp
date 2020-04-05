/*
 * CCtpMiniTrader.cpp
 *
 *  Created on: 2017年12月21日
 *      Author: hongxu
 */

#include "CCtpMiniTrader.h"
#include "SimpleLogger.h"


CCtpMiniTrader::CCtpMiniTrader()
{

}

CCtpMiniTrader::~CCtpMiniTrader()
{

}

bool CCtpMiniTrader::init()
{
	if(p_api_ ==  NULL)
	{
		CConfig *p_cnf = CGlobalParameter::getConfig();

		broker_id_ = p_cnf->getVal<string>("CTPMINI", "broker_id");
		user_id_ = p_cnf->getVal<string>("CTPMINI", "user_id");

		string flow_path = p_cnf->getVal<string>("CTPMINI", "trade_flow_path");
		CThostFtdcTraderApi *api = CThostFtdcTraderApi::CreateFtdcTraderApi(flow_path.c_str());
		p_api_ = api;

		api->RegisterSpi(this);

		vector<string> front_list;
		p_cnf->getValList("CTPMINI", "front", front_list);
		for(auto& it: front_list) api->RegisterFront((char*)it.c_str());

		api->SubscribePrivateTopic(THOST_TERT_QUICK);
		api->SubscribePublicTopic(THOST_TERT_QUICK);
		api->Init();
		return true;
	}

	LOG_ERR("CCtpMiniTrader is already inited");
	return false;
}

void CCtpMiniTrader::reqUserLogin()
{
	CConfig *p_cnf = CGlobalParameter::getConfig();

	CThostFtdcReqUserLoginField login_req;
	memset(&login_req, 0, sizeof(login_req));

	strcpy(login_req.BrokerID, broker_id_.c_str());
	strcpy(login_req.UserID, user_id_.c_str());

	string pass = p_cnf->getVal<string>("CTPMINI", "passwd");
	strcpy(login_req.Password, pass.c_str());
	LOG_TRACE("CCtpMiniTrader: broker_id=[%s], user_id=[%s], passwd=[%c*****]", broker_id_.c_str(), user_id_.c_str(), pass[0]);

	int ret = p_api_->ReqUserLogin(&login_req, ++request_id_);
	parseRetCode(ret, "login");
}

