#include "IOMonitorHelper.h"
#include "OTMonitorHelper.h"
#include "MurmurHash2.h"
#include "ATStructure.h"
#include "CTradeBaseInfo.h"
#include "RiskStg.h"
#include "RiskTop.h"
#include <fstream>
#include "Logger.h"

json readJsonContent(string path)
{
	ifstream file(path);
	if (! file)
	{
		ALERT("can't read file: %s", path.c_str());
		return nullptr;
	}
	string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	json j;
	try 
	{
		j = json::parse(content);
	}
	catch (...)
	{
		ALERT("can't parse json file: %s", path.c_str());
		return nullptr;
	}
	
	return j;
}

void simulateExecution(int dir, int off, double px, int vol, uint32_t instr_hash, RiskStg* p_risk)
{
	tOrderTrack* p_ord_trk = new tOrderTrack;
	p_ord_trk->instr_hash = instr_hash;
	p_ord_trk->status |= ODS(TDSEND);
	p_ord_trk->dir = dir;
	p_ord_trk->off = off;
	p_ord_trk->price = px;
	p_ord_trk->vol = vol;
	p_risk->onNew(p_ord_trk->dir, p_ord_trk->off, p_ord_trk->price, p_ord_trk->vol, instr_hash, 0);
	
	p_ord_trk->status |= ODS(EXECUTION);
	p_ord_trk->vol_traded = vol;
	tRtnMsg* p_rtn_msg = new tRtnMsg;
	p_rtn_msg->instr_hash = instr_hash;
	p_rtn_msg->msg_type = ODS(EXECUTION);
	p_rtn_msg->price = px;
	p_rtn_msg->vol = vol;
	p_rtn_msg->dir = p_ord_trk->dir;
	p_rtn_msg->off = p_ord_trk->off;
	p_risk->onRtn(p_ord_trk, p_rtn_msg);
}

bool testOnRtnSeries()
{
	LOG_DBG("======== test rtn send->accept->market_accept->execution->canceled ========");
	string path = "risk.json";
	string instr("rb1810");
	string prd("rb");
	uint32_t instr_hash = INSTR_STR_TO_HASH(instr);
	uint32_t prd_hash = INSTR_STR_TO_HASH(prd);
	tInstrumentInfo* p_instr_info = &CTradeBaseInfo::instr_info_[instr_hash];
	p_instr_info->instr_hash = instr_hash;
	strcpy(p_instr_info->instr, instr.c_str());
	p_instr_info->product_hash = prd_hash;
	strcpy(p_instr_info->product, prd.c_str());
	p_instr_info->vol_multiple = 10;
	json j = readJsonContent(path);
	RiskStg* p_risk = new RiskStg;
	
	LOG_DBG("======== 20180814 ========");
	CTradeBaseInfo::trading_day_ = "20180814";
	if (! p_risk->init("test_rtn_stg", j))
	{
		ALERT("init risk fail.");
		return false;
	}
	p_risk->load();
	
	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4339.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseToday, 4338.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4344.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_Open, 4345.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4345.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_Open, 4344.0, 10, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4345.0, 10, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_Open, 4346.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_Open, 4347.0, 1, instr_hash, p_risk);

	p_risk->onTickPx(instr_hash, 4341);
	p_risk->save(CTradeBaseInfo::trading_day_, true);
	
	LOG_DBG("======== 20180815 ========");
	CTradeBaseInfo::trading_day_ = "20180815";
	p_risk->onSwitchDay();

	p_risk->onTickPx(instr_hash, 4344);
	p_risk->save(CTradeBaseInfo::trading_day_, true);
	
	LOG_DBG("======== 20180816 ========");
	CTradeBaseInfo::trading_day_ = "20180816";
	p_risk->onSwitchDay();

	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4334.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_Open, 4332.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4330.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4329.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4329.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4328.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4328.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4329.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4329.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4329.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4331.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4332.0, 3, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4333.0, 3, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_CloseYesterday, 4332.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Buy, AT_CHAR_Open, 4333.0, 2, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4333.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseYesterday, 4333.0, 1, instr_hash, p_risk);
	simulateExecution(AT_CHAR_Sell, AT_CHAR_CloseToday, 4331.0, 1, instr_hash, p_risk);

	p_risk->onTickPx(instr_hash, 4324);
	p_risk->onSwitchDay();
	
	return true;
}

int main(int argc, char* argv[]) 
{
	initLogger("log.cnf");

	if (! testOnRtnSeries())
	{
		ALERT("test on rtn series fail.");
		return 1;
	}
	
	return 0;
}
