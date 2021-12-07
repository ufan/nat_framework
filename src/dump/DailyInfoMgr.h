/**
 * @file      DailyInfoMgr.h
 * @brief     Header of DailyInfoMgr
 * @date      Tue Dec  7 19:22:20 2021
 * @author    Yong
 * @copyright BSD-3-Clause
 *
 * This class is used by CTPDumper to dump the statistics of today's available
 * trading instruments to JSON file.
 *
 * The JSON file has the name convension as CTP_{trade_date}_daily.info.
 * There three top fields inside the JSON file:
 * - trading_date: the date to which this file matches
 * - instrument_list: the list of the basic info of trading contracts
 * - statistic: the list of statistics data of these trading contracts
 *
 * The basic data is filled once by CCTPTD (request instruments).
 * The statistic data is updated at each tick by CCTPMD (on response of the
 * subscribed instruments).
 */

#ifndef DUMP_DAILYINFOMGR_H
#define DUMP_DAILYINFOMGR_H
#include <unordered_map>

#include "ThostFtdcMdApi.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class DailyInfoMgr {
 public:
  static bool init(json& j, const char* trading_date);
  static bool release();
  static bool write();
  static void onTick(CThostFtdcDepthMarketDataField* pDepthMarketData);

  static json j;
  static char trading_date[16];
  static char daily_info_path[256];
  static unordered_map<string, int> map_instr_cnt;
};

#endif
