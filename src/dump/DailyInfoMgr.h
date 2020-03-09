#ifndef DUMP_DAILYINFOMGR_H
#define DUMP_DAILYINFOMGR_H
#include "json.hpp"
#include "ThostFtdcMdApi.h"
#include <unordered_map>

using namespace std;
using json = nlohmann::json;

class DailyInfoMgr
{
public:
	static bool init(json& j, const char* trading_date);
	static bool release();
	static bool write();
	static void onTick(CThostFtdcDepthMarketDataField *pDepthMarketData);
	
	static json j;
	static char trading_date[16];
	static char daily_info_path[256];
	static unordered_map<string, int> map_instr_cnt;
};

#endif