/*
 * CMDEngineCtp.cpp
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#include "CMDEngineCtp.h"
#include "ATStructure.h"
#include "SysConf.h"
#include "IOCommon.h"
#include "compiler.h"
#include "MurmurHash2.h"
#include "utils.h"


CMDEngineCtp::CMDEngineCtp() : p_api_(NULL), reqid_(0)
{

}

CMDEngineCtp::~CMDEngineCtp()
{

}

bool CMDEngineCtp::init()
{
	name_ = config_["/CTPMD/name"_json_pointer];

	string con_dir = config_["/CTPMD/con_dir"_json_pointer];
	if(!createPath(con_dir.data())){
		LOG_ERR("create dir %s err", con_dir.data());
		return false;
	}
	p_api_ = CThostFtdcMdApi::CreateFtdcMdApi(con_dir.c_str(), false);

	p_api_->RegisterSpi(this);

	auto& front = config_["/CTPMD/front"_json_pointer];
	for(auto& i : front)
	{
		p_api_->RegisterFront((char*)i.get<string>().c_str());
	}
	return true;
}

bool CMDEngineCtp::start()
{
	long timeout = config_["/CTPMD/timeout"_json_pointer];
	p_api_->Init();
	long end = time(NULL) + timeout;
	while(!(volatile bool)is_login_ && time(NULL) < end) usleep(200000);  // 200ms
	return is_login_;
}

void CMDEngineCtp::join()
{
	p_api_->Join();
}

void CMDEngineCtp::release()
{
	if(p_api_)
	{
		p_api_->RegisterSpi(NULL);
		p_api_->Release();
		p_api_ = nullptr;
	}
	is_login_ = false;
}

void CMDEngineCtp::subscribe(const vector<string> &instr)
{
	// subscribe instrument
	if(instr.size() > 0)
	{
		char **inst = new char*[instr.size()];
		for(uint32_t i = 0; i < instr.size(); ++i)
		{
			inst[i] = (char*)instr[i].c_str();
			ENGLOG("subscribe %s", inst[i]);
		}
		int ret = p_api_->SubscribeMarketData(inst, (int)instr.size());
		delete [] inst;

		if(ret != 0)
		{
			ALERT("SubscribeMarketData err: %d", ret);
		}
	}
}

void CMDEngineCtp::unsubscribe(const vector<string> &instr)
{
	// subscribe instrument
	if(instr.size() > 0)
	{
		char **inst = new char*[instr.size()];
		for(uint32_t i = 0; i < instr.size(); ++i)
		{
			inst[i] = (char*)instr[i].c_str();
			ENGLOG("unsubscribe %s", inst[i]);
		}
		int ret = p_api_->UnSubscribeMarketData(inst, (int)instr.size());
		delete [] inst;

		if(ret != 0)
		{
			ALERT("UnSubscribeMarketData err: %d", ret);
		}
	}
}

void CMDEngineCtp::OnFrontConnected()
{
	ENGLOG("front connected.");

	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, config_["/CTPMD/BrokerID"_json_pointer].get<string>().c_str());
	strcpy(req.UserID, config_["/CTPMD/UserID"_json_pointer].get<string>().c_str());
	strcpy(req.Password, config_["/CTPMD/Password"_json_pointer].get<string>().c_str());

	int ret = p_api_->ReqUserLogin(&req, reqid_++);
	if(ret != 0)
	{
		ALERT("login err:%d", ret);
		exit(-1);
	}
}

void CMDEngineCtp::OnFrontDisconnected(int nReason)
{
	ALERT("front disconnected, code:%d. exit...", nReason);
	stop();
}

void CMDEngineCtp::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID,
		bool bIsLast)
{
	ENGLOG("OnRspError %d: %s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
}

void CMDEngineCtp::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	is_login_ = true;
	ENGLOG("login succ.");
}

void CMDEngineCtp::OnRspSubMarketData(
		CThostFtdcSpecificInstrumentField* pSpecificInstrument,
		CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(pRspInfo->ErrorID != 0)
	{
		ALERT("Subscribe market data err %d: %s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		exit(-1);
	}
}

void CMDEngineCtp::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* p_data)
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

