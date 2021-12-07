/**
 * @file      CCTPTD.h
 * @brief     Header of CCTPTD
 * @date      Tue Dec  7 20:22:34 2021
 * @author    Yong
 * @copyright BSD-3-Clause
 *
 * This class provides an implementation of TD, whose only function is to
 * retrieve the list of all available instruments on the market.
 * This list is then used by CCTPMD to subscribe the quotes of each instrument.
 * And it is also later dumped onto disk files by DailyInfoMgr (for summary JSON
 * file) and FileMgr (for binary tick data).
 */

#ifndef CCTPTD_H
#define CCTPTD_H

#include <string>
#include <vector>

#include "Structure.h"
#include "ThostFtdcTraderApi.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class CCTPTD : public CThostFtdcTraderSpi {
 public:
  ~CCTPTD() {}
  bool init(json &j);
  bool release();
  bool login(json &j);
  bool confirm(json &j);
  bool qryInstrument();
  bool printResult(const char *act, int ret);

  void OnFrontConnected();
  void OnFrontDisconnected(int nReason);
  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                      CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                      bool bIsLast);
  void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                       CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                       bool bIsLast);
  void OnRspSettlementInfoConfirm(
      CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
      CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
  void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
                          CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                          bool bIsLast);

  char trading_date[16] = {0};
  vector<Instrument>
      vec_instr;  // list of basic information of traded instruments from market
  CThostFtdcTraderApi *api = nullptr;
  int request_id = 0;
  int status = 0;  // status code for TD event loop
};

#endif
