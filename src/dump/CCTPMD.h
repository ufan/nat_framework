#ifndef DUMP_CCTPMD_H
#define DUMP_CCTPMD_H

#include "ThostFtdcMdApi.h"
#include "Structure.h"
#include "json.hpp"
#include <string>

using namespace std;
using json = nlohmann::json;

class CCTPMD : public CThostFtdcMdSpi
{
public:
	bool init(json& j);
	bool release();
	bool login(json& j);
	bool regInstrument(vector<Instrument>& vec_instr);
	bool isInTradingTime();
	bool printResult(const char *act, int ret);

	void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
	
	char trading_date[16] = {0};
	CThostFtdcMdApi *api = nullptr;
	int request_id = 0;
	int status = 0;
};

#endif