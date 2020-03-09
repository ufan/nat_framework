#include "CCTPMD.h"
#include "utils.h"
#include "CEncodeConv.h"
#include "CTimer.h"
#include "unistd.h"
#include "FileMgr.h"
#include "DailyInfoMgr.h"
#include "float.h"
#include "Logger.h"

bool CCTPMD::init(json& j)
{
	ENGLOG("ctp md init.\n");
	if (! createPath(j["flow_path"].get<string>()))
	{
		ALERT("can't create path %s\n", j["flow_path"].get<string>().c_str());
		return false;
	}
	if (api == nullptr)
	{
		api = CThostFtdcMdApi::CreateFtdcMdApi(j["flow_path"].get<string>().c_str());
		if (!api)
		{
			ALERT("failed in creating md api!\n");
			return false;
		}
		api->RegisterSpi(this);
		for (string addr : j["md_uri"]) {
			api->RegisterFront((char*)addr.c_str());
			ENGLOG("register md front %s.\n", addr.c_str());
		}
		api->Init();
	}
	
	return true;
}

bool CCTPMD::release()
{
	ENGLOG("ctp md release.\n");
	if (api) {
		api->RegisterSpi(NULL);
		api->Release();
		api = nullptr;
	}
	status = 5;
	return true;
}

bool CCTPMD::login(json& j)
{
	CThostFtdcReqUserLoginField field;
	memset(&field, 0, sizeof(field));
	strcpy(field.BrokerID, j["Account"][0]["BrokerID"].get<string>().c_str());
	strcpy(field.UserID, j["Account"][0]["UserID"].get<string>().c_str());
	strcpy(field.Password, j["Account"][0]["Password"].get<string>().c_str());
	int ret = api->ReqUserLogin(&field, ++request_id);
	return printResult("user login", ret);
}

bool CCTPMD::regInstrument(vector<Instrument>& vec_instr)
{
	char** arr_instr_str = new char*[vec_instr.size()];
	for (int i = 0; i < vec_instr.size(); ++i)
	{
		arr_instr_str[i] = new char[31];
		memset(arr_instr_str[i], 0, 31);
		strcpy(arr_instr_str[i], vec_instr[i].instr_str);
	}
	int ret = api->SubscribeMarketData(arr_instr_str, vec_instr.size());
	for (int i = 0; i < vec_instr.size(); ++i)
	{
		delete[] arr_instr_str[i];
	}
	delete[] arr_instr_str;
	status = 3;
	return printResult("register instruments", ret);
}

bool CCTPMD::isInTradingTime()
{
	long nano = CTimer::instance().getNano();
	int hr = (nano / 1000000000 / 3600 + 8) % 24;
	int mi = nano / 1000000000 / 60 % 60;
	int se = nano / 1000000000 % 60;
	int hrmise = hr * 3600 + mi * 60 + se;
	
	// 3:00~8:00, 16:00~20:00 offhour
	if ((3*3600 <= hrmise && hrmise < 8*3600) || (16*3600 <= hrmise && hrmise < 20*3600))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CCTPMD::printResult(const char *act, int ret)
{
	switch (ret) {
		case 0:
			ENGLOG("succeed in %s req.\n", act);
			break;
		case -1:
			ALERT("failed in %s req, network error.\n", act);
			break;
		case -2:
			ALERT("failed in %s req, queue quantity exceed.\n", act);
			break;
		case -3:
			ALERT("failed in %s req, intensity exceed.\n", act);
			break;
	}
	return ret == 0;
}

void CCTPMD::OnFrontConnected()
{
	ENGLOG("md front connected.\n");
	status = 1;
}

void CCTPMD::OnFrontDisconnected(int nReason)
{
	ALERT("md front disconnected, reason=%d.\n", nReason);
	release();
	status = 0;
}

void CCTPMD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "user login rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
	}
	else
	{
		if (bIsLast)
		{
			strcpy(trading_date, pRspUserLogin->TradingDay);
			status = 2;
			ENGLOG("succeed in %s. trading_date=%s\n", action, trading_date);
		}
	}
}

void CCTPMD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "user logout rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
	}
	else
	{
		if (bIsLast)
		{
			status = 1;
			ENGLOG("succeed in %s.\n", action);
		}
	}
}

void CCTPMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	MarketDataHead md_head;
	md_head.local_time = CTimer::instance().getNano();
	fwrite(&md_head, sizeof(MarketDataHead), 1, FileMgr::pf_all);
	fwrite(pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField), 1, FileMgr::pf_all);
	FILE *p_file = FileMgr::getFilePtr(pDepthMarketData->InstrumentID);
	fwrite(&md_head, sizeof(MarketDataHead), 1, p_file);
	fwrite(pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField), 1, p_file);
	
	DailyInfoMgr::onTick(pDepthMarketData);
}
