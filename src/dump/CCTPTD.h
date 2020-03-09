#ifndef CCTPTD_H
#define CCTPTD_H

#include "ThostFtdcTraderApi.h"
#include "Structure.h"
#include "json.hpp"
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

class CCTPTD : public CThostFtdcTraderSpi
{
public:
	~CCTPTD() {}
	bool init(json& j);
	bool release();
	bool login(json& j);
	bool confirm(json& j);
	bool qryInstrument();
	bool printResult(const char *act, int ret);
	
	void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	char trading_date[16] = {0};
	vector<Instrument> vec_instr;
	CThostFtdcTraderApi* api = nullptr;
	int request_id = 0;
	int status = 0;
};

#endif