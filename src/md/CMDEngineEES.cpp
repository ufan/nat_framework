/*
 * CMDEngineEES.cpp
 *
 *  Created on: May 22, 2018
 *      Author: hongxu
 */

#include "CMDEngineEES.h"
#include "ATStructure.h"
#include "IOCommon.h"
#include "MurmurHash2.h"
#include "utils.h"

CMDEngineEES::CMDEngineEES()
{

}

CMDEngineEES::~CMDEngineEES()
{

}

bool CMDEngineEES::init()
{
	name_ = config_["/EESMD/name"_json_pointer];
	api_ = CreateEESQuoteApi();
	return true;
}

bool CMDEngineEES::start()
{
	bool use_multicast = config_["/EESMD/use_multicast"_json_pointer];
	if(use_multicast)
	{
		if(!initMulticast()) return false;
	}
	else
	{
		if(!connectTcp()) return false;
	}
	long timeout = config_["/EESMD/timeout"_json_pointer];
	long end = time(NULL) + timeout;
	while(!(volatile bool)is_login_ && time(NULL) < end) usleep(200000);  // 200ms
	return is_login_;
}

void CMDEngineEES::join()
{

}

void CMDEngineEES::release()
{
	if(api_)
	{
		if(is_login_)
		{
			api_->DisConnServer();
			is_login_ = false;
		}
		DestroyEESQuoteApi(api_);
		api_ = nullptr;
	}
}

void CMDEngineEES::subscribe(const vector<string> &instr)
{
	for(auto &i : instr)
	{
		api_->RegisterSymbol(EQS_FUTURE, i.c_str());
		ENGLOG("subscribe %s", i.c_str());
	}
}

void CMDEngineEES::unsubscribe(const vector<string> &instr)
{
	for(auto &i : instr)
	{
		api_->UnregisterSymbol(EQS_FUTURE, i.c_str());
		ENGLOG("unsubscribe %s", i.c_str());
	}
}

bool CMDEngineEES::connectTcp()
{
    string query_ip = config_["/EESMD/md_ip"_json_pointer];
    int query_port = config_["/EESMD/md_port"_json_pointer];

    EqsTcpInfo tcpinfo;
    memset(&tcpinfo, 0, sizeof(tcpinfo));
    strcpy(tcpinfo.m_eqsIp, query_ip.c_str());
    tcpinfo.m_eqsPort = query_port;
    vector<EqsTcpInfo> vecEti;
    vecEti.push_back(tcpinfo);

	bool ret = api_->ConnServer(vecEti, this);
	if(!ret)
	{
		ALERT("connect to EES server failed!\n");
		return false;
	}
	return true;
}

bool CMDEngineEES::initMulticast()
{
	EqsMulticastInfo info;
	string mc_ip = config_["/EESMD/multicast_config/mc_ip"_json_pointer];
	int mc_port = config_["/EESMD/multicast_config/mc_port"_json_pointer];
	string local_ip = config_["/EESMD/multicast_config/local_ip"_json_pointer];
	int local_port = config_["/EESMD/multicast_config/local_port"_json_pointer];
	string exchange = config_["/EESMD/multicast_config/exchange"_json_pointer];

	strcpy(info.m_mcIp, mc_ip.c_str());
	info.m_mcPort = mc_port;
	strcpy(info.m_mcLoacalIp, local_ip.c_str());
	info.m_mcLocalPort = local_port;
	strcpy(info.m_exchangeId, exchange.c_str());

	vector<EqsMulticastInfo> vecEmi;
	vecEmi.push_back(info);

	bool ret = api_->InitMulticast(vecEmi, this);
	if(!ret)
	{
		ALERT("init multicast failed!\n");
		return false;
	}
	return true;
}

void CMDEngineEES::OnEqsConnected()
{
	ENGLOG("EES Connected.");
	EqsLoginParam login_param;
	string login_id = config_["/EESMD/login_id"_json_pointer];
	string passwd = config_["/EESMD/passwd"_json_pointer];
	strcpy(login_param.m_loginId, login_id.c_str());
	strcpy(login_param.m_password, passwd.c_str());
	ENGLOG("try login for login_id:%s", login_id.c_str());
	api_->LoginToEqs(login_param);
}

void CMDEngineEES::OnEqsDisconnected()
{
	ALERT("front disconnected, exit...");
	stop();
}

void CMDEngineEES::OnLoginResponse(bool bsuccess, const char* pReason)
{
	if(bsuccess)
	{
		is_login_ = true;
		ENGLOG("EES login success.");
	}
	else
	{
		ALERT("EES login failed: %s.", pReason);
		is_login_ = false;
	}
}

void CMDEngineEES::OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char *pSymbol, bool bSuccess)
{
	if(bSuccess)
	{
		ENGLOG("subscribe %s success.", pSymbol);
	}
	else
	{
		ENGLOG("subscribe %s failed.", pSymbol);
	}
}

void CMDEngineEES::OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess)
{
	if(bSuccess)
	{
		ENGLOG("unsubscribe %s success.", pSymbol);
	}
	else
	{
		ENGLOG("unsubscribe %s failed.", pSymbol);
	}
}

void CMDEngineEES::OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* p_data)
{
	tIOMarketData *io = (tIOMarketData*)md_writer_.prefetch(sizeof(tIOMarketData));
	io->cmd = IO_MARKET_DATA;
	UnitedMarketData &p = io->market_data;
	p.instr_hash = INSTR_NAME_TO_HASH(p_data->InstrumentID);
	p.last_px = p_data->LastPrice;
	p.cum_vol = p_data->Volume;
	p.cum_turnover = p_data->Turnover;
	p.avg_px = p_data->AveragePrice;
	p.ask_px = p_data->AskPrice1;
	p.bid_px = p_data->BidPrice1;
	p.ask_vol = p_data->AskVolume1;
	p.bid_vol = p_data->BidVolume1;
	p.open_interest = p_data->OpenInterest;
	memcpy(p.instr_str, p_data->InstrumentID, sizeof(p.instr_str));
	p.exch_time = (getSecondsFromClockStr(p_data->UpdateTime) + CTimer::instance().getDayBeginTime()) * 1000000000L + p_data->UpdateMillisec * 1000000L;
	if(day_night_mode_ == MODE_NIGHT) {if(p_data->UpdateTime[0] == '0') p.exch_time += 86400L * 1000000000L;}
	else {if(p_data->UpdateTime[0] == '2') p.exch_time -= 86400L * 1000000000L;}
	md_writer_.commit();
}

