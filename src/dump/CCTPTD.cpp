#include "CCTPTD.h"
#include "utils.h"
#include "CEncodeConv.h"
#include "Logger.h"
#include "DailyInfoMgr.h"

bool CCTPTD::init(json& j)
{
	ENGLOG("ctp td init.\n");
	if (! createPath(j["flow_path"].get<string>()))
	{
		ALERT("can't create path %s\n", j["flow_path"].get<string>().c_str());
		return false;
	}
	if (api == nullptr)
	{
		api = CThostFtdcTraderApi::CreateFtdcTraderApi(j["flow_path"].get<string>().c_str());
		if (!api)
		{
			ALERT("failed in creating td api!\n");
			return false;
		}
		api->RegisterSpi(this);
		for (string addr : j["td_uri"]) {
			api->RegisterFront((char*)addr.c_str());
			ENGLOG("register td front %s.\n", addr.c_str());
		}
		api->SubscribePublicTopic(THOST_TERT_RESTART);
		api->SubscribePrivateTopic(THOST_TERT_QUICK);
		api->Init();
	}
	
	return true;
}

bool CCTPTD::release()
{
	ENGLOG("ctp td release.\n");
	if (api) {
		api->RegisterSpi(NULL);
		api->Release();
		api = nullptr;
	}
	status = 5;
	return true;
}

bool CCTPTD::login(json& j)
{
	CThostFtdcReqUserLoginField field;
	strcpy(field.BrokerID, j["Account"][0]["BrokerID"].get<string>().c_str());
	strcpy(field.UserID, j["Account"][0]["UserID"].get<string>().c_str());
	strcpy(field.Password, j["Account"][0]["Password"].get<string>().c_str());
	int ret = api->ReqUserLogin(&field, ++request_id);
	return printResult("user login", ret);
}

bool CCTPTD::confirm(json& j)
{
	CThostFtdcSettlementInfoConfirmField field;
	memset(&field, 0, sizeof(field));
	strcpy(field.BrokerID, j["Account"][0]["BrokerID"].get<string>().c_str());
	strcpy(field.InvestorID, j["Account"][0]["UserID"].get<string>().c_str());
	int ret = api->ReqSettlementInfoConfirm(&field, ++request_id);
	return printResult("settlement info confirm", ret);
}

bool CCTPTD::qryInstrument()
{
	vec_instr.clear();
	CThostFtdcQryInstrumentField field;
	memset(&field, 0, sizeof(field));
	int ret = api->ReqQryInstrument(&field, ++request_id);
	return printResult("query instruments", ret);
}

bool CCTPTD::printResult(const char *act, int ret) {
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

void CCTPTD::OnFrontConnected()
{
	ENGLOG("td front connected.\n");
	status = 1;
}

void CCTPTD::OnFrontDisconnected(int nReason)
{
	ALERT("td front disconnected, reason=%d.\n", nReason);
	release();
	status = 0;
}

void CCTPTD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "user login rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
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

void CCTPTD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "user logout rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
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

void CCTPTD::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "settlement info confirm rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
	}
	else
	{
		if (bIsLast)
		{
			status = 3;
			ENGLOG("succeed in %s.\n", action);
		}
	}
}

void CCTPTD::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	const char* action = "query instruments rsp";
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
	{
		ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID, CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
	}
	else
	{
		if (pInstrument)
		{
			Instrument instr;
			strcpy(instr.instr_str, pInstrument->InstrumentID);
			strcpy(instr.exch_str, pInstrument->ExchangeID);
			strcpy(instr.prd_str, pInstrument->ProductID);
			switch (pInstrument->ProductClass) {
				case THOST_FTDC_PC_Futures:
					instr.prd_cls = ProductClassType::PC_Futures;
					break;
				case THOST_FTDC_PC_Options:
					instr.prd_cls = ProductClassType::PC_Options;
					break;
				case THOST_FTDC_PC_Combination:
					instr.prd_cls = ProductClassType::PC_Combination;
					break;
				case THOST_FTDC_PC_Spot:
					instr.prd_cls = ProductClassType::PC_Spot;
					break;
				case THOST_FTDC_PC_EFP:
					instr.prd_cls = ProductClassType::PC_EFP;
					break;
				case THOST_FTDC_PC_SpotOption:
					instr.prd_cls = ProductClassType::PC_SpotOption;
					break;
				default:
					break;
			}
			instr.volume_multiple = pInstrument->VolumeMultiple;
			instr.price_tick = pInstrument->PriceTick;
			strcpy(instr.underlying_instr_str, pInstrument->UnderlyingInstrID);
			strcpy(instr.open_date, pInstrument->OpenDate);
			strcpy(instr.expire_date, pInstrument->ExpireDate);
			instr.strike_px = pInstrument->StrikePrice;
			instr.opt_type = pInstrument->OptionsType == THOST_FTDC_CP_CallOptions ? OptionsType::OT_Call : OptionsType::OT_Put;
			vec_instr.emplace_back(instr);

			json& j = DailyInfoMgr::j["instrument_list"][string(pInstrument->InstrumentID)];
			j["exch"] = pInstrument->ExchangeID;
			j["prd"] = pInstrument->ProductID;
			j["type"] = pInstrument->ProductClass;
			j["volume_multiple"] = pInstrument->VolumeMultiple;
			j["price_tick"] = pInstrument->PriceTick;
			j["open_date"] = pInstrument->OpenDate;
			j["expire_date"] = pInstrument->ExpireDate;
			j["underlying_instr"] = pInstrument->UnderlyingInstrID;
			j["strike_px"] = pInstrument->StrikePrice;
			j["options_type"] = pInstrument->OptionsType;
			
			DailyInfoMgr::map_instr_cnt[string(pInstrument->InstrumentID)] = 0;
		}
		if (bIsLast)
		{
			status = 4;
			ENGLOG("succeed in %s.\n", action);
		}
	}
}
