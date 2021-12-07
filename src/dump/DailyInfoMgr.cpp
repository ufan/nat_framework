#include "DailyInfoMgr.h"

#include <fstream>

#include "Logger.h"
#include "float.h"
#include "utils.h"

using namespace std;

json DailyInfoMgr::j;
char DailyInfoMgr::trading_date[16] = {0};
char DailyInfoMgr::daily_info_path[256] = {0};
unordered_map<string, int> DailyInfoMgr::map_instr_cnt;

/**
 * @brief create the output directory
 */
bool DailyInfoMgr::init(json& j, const char* trading_date) {
  ENGLOG("DailyInfoMgr init.\n");
  strcpy(daily_info_path, j["daily_info_path"].get<string>().c_str());
  strcpy(DailyInfoMgr::trading_date, trading_date);
  return createPath(daily_info_path);
}

bool DailyInfoMgr::release() {
  ENGLOG("DailyInfoMgr release.\n");
  return write();
}

/**
 * @brief Flush the available data onto disk file, whatever it has received.
 */
bool DailyInfoMgr::write() {
  ENGLOG("DailyInfoMgr write.\n");
  if (trading_date != nullptr && strlen(trading_date) != 0) {
    string path =
        string(daily_info_path) + "/CTP_" + trading_date + "_daily.info";
    ofstream file(path, ios::out);
    if (!file) {
      ALERT("can't write file: %s\n", path.c_str());
      return false;
    }

    j["trading_date"] = trading_date;

    file << std::setw(4) << j << endl;
    file.close();
  }
  return true;
}

/**
 * @brief Update the statisics field. Invoked by CCTPMD.
 */
void DailyInfoMgr::onTick(CThostFtdcDepthMarketDataField* pDepthMarketData) {
  ++DailyInfoMgr::map_instr_cnt[string(pDepthMarketData->InstrumentID)];

  if (pDepthMarketData->SettlementPrice != 0 &&
      pDepthMarketData->SettlementPrice != DBL_MAX) {
    DailyInfoMgr::j["statistic"][string(pDepthMarketData->InstrumentID)]
                   ["settlement_price"] = pDepthMarketData->SettlementPrice;
    DailyInfoMgr::j["statistic"][string(pDepthMarketData->InstrumentID)]
                   ["open_interest"] = pDepthMarketData->OpenInterest;
    DailyInfoMgr::j["statistic"][string(pDepthMarketData->InstrumentID)]
                   ["cum_vol"] = pDepthMarketData->Volume;
    DailyInfoMgr::j["statistic"][string(pDepthMarketData->InstrumentID)]
                   ["cum_turnover"] = pDepthMarketData->Turnover;
    DailyInfoMgr::j["statistic"][string(
        pDepthMarketData->InstrumentID)]["tick_cnt"] =
        DailyInfoMgr::map_instr_cnt[string(pDepthMarketData->InstrumentID)];
  }
}
