#include "CCTPTD.h"

#include "CEncodeConv.h"
#include "DailyInfoMgr.h"
#include "Logger.h"
#include "utils.h"

bool CCTPTD::init(json &j) {
  ENGLOG("ctp td init.\n");
  if (!createPath(j["flow_path"].get<string>())) {
    ALERT("can't create path %s\n", j["flow_path"].get<string>().c_str());
    return false;
  }
  if (api == nullptr) {
    // step 1: create TraderApi
    api = CThostFtdcTraderApi::CreateFtdcTraderApi(
        j["flow_path"].get<string>().c_str());
    if (!api) {
      ALERT("failed in creating td api!\n");
      return false;
    }

    // step 2: register the event handler object, in this case: this instance
    api->RegisterSpi(this);

    // step 3: register trade fronts. Multi-fronts can be registered, but
    // TraderApi will select one of them for connection. (Randomly?)
    for (string addr : j["td_uri"]) {
      api->RegisterFront((char *)addr.c_str());
      ENGLOG("register td front %s.\n", addr.c_str());
    }

    // step 4: register the public and private flows
    api->SubscribePublicTopic(
        THOST_TERT_RESTART);  // from the first message of the day
    api->SubscribePrivateTopic(
        THOST_TERT_QUICK);  // from the current message flow

    // step 5: TraderApi initialization, in which it tries to connect to Thost
    // through one of the registered trader front. If the connection is
    // successful, the OnFrontConnected hook from the event handler object
    // will be invoked by TraderApi.
    api->Init();
  }

  return true;
}

bool CCTPTD::release() {
  ENGLOG("ctp td release.\n");
  if (api) {
    api->RegisterSpi(NULL);
    api->Release();
    api = nullptr;
  }
  status = 5;
  return true;
}

bool CCTPTD::login(json &j) {
  CThostFtdcReqUserLoginField field;
  strcpy(field.BrokerID, j["Account"][0]["BrokerID"].get<string>().c_str());
  strcpy(field.UserID, j["Account"][0]["UserID"].get<string>().c_str());
  strcpy(field.Password, j["Account"][0]["Password"].get<string>().c_str());
  int ret = api->ReqUserLogin(&field, ++request_id);
  return printResult("user login", ret);
}

/**
 * @brief Send request to confirm today's settlement, which is needed at least
once each day.
 *
 * Normally, at the beginning of each market day (i.e. before putting an order),
 * an user should first check his settlement information (ReqQrySettlementInfo)
 * and then confirm it (ReqSettlementInfoConfirm). User can check whether he has
 * confirmed the settlement by ReqQrySettlementInfoConfirm.
 *
 * Here, we always confirm it every time we conncet to the Thost, and without
 * checking.
 */
bool CCTPTD::confirm(json &j) {
  CThostFtdcSettlementInfoConfirmField field;
  memset(&field, 0, sizeof(field));
  strcpy(field.BrokerID, j["Account"][0]["BrokerID"].get<string>().c_str());
  strcpy(field.InvestorID, j["Account"][0]["UserID"].get<string>().c_str());
  int ret = api->ReqSettlementInfoConfirm(&field, ++request_id);
  return printResult("settlement info confirm", ret);
}

/**
 * @brief Send request for all available instruments
 */
bool CCTPTD::qryInstrument() {
  vec_instr.clear();

  // field 0 means request all available instruments
  CThostFtdcQryInstrumentField field;
  memset(&field, 0, sizeof(field));

  int ret = api->ReqQryInstrument(&field, ++request_id);
  return printResult("query instruments", ret);
}

/**
 * @brief Summary
 * @details Logging output based on the return code from the request command.
 * @param[in] act Description
 * @param[in] ret 0: request sent successfully,
 *                -1: failed due to network problem,
 *                -2,-3: failed due to Thost limitation
 * @return true for success, false for failure
 */
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

void CCTPTD::OnFrontConnected() {
  ENGLOG("td front connected.\n");
  status = 1;
}

void CCTPTD::OnFrontDisconnected(int nReason) {
  ALERT("td front disconnected, reason=%d.\n", nReason);
  release();
  status = 0;
}

void CCTPTD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                            CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                            bool bIsLast) {
  const char *action = "user login rsp";
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID,
          CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  } else {
    if (bIsLast) {
      strcpy(trading_date, pRspUserLogin->TradingDay);
      status = 2;
      ENGLOG("succeed in %s. trading_date=%s\n", action, trading_date);
    }
  }
}

void CCTPTD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
                             CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                             bool bIsLast) {
  const char *action = "user logout rsp";
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID,
          CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  } else {
    if (bIsLast) {
      status = 1;
      ENGLOG("succeed in %s.\n", action);
    }
  }
}

void CCTPTD::OnRspSettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  const char *action = "settlement info confirm rsp";
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID,
          CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  } else {
    if (bIsLast) {
      status = 3;
      ENGLOG("succeed in %s.\n", action);
    }
  }
}

/**
 * @brief Receive the queried instrument information from Thost.
          Fill in the instrument vector and daily_info JSON basic info field.

 * @param[inout] pInstrument Description
 * @param[out] pRspInfo contains error id and error message
 * @param[in] nRequestID same as the request id in qryInstrument
 * @param[in] bIsLast A query of all available instruments can be requested, but
 *            each time only one instrument is returned from Thost. This is the
 flag
 *            indicating whether this is the last instrument of the returned
 message list.
 */
void CCTPTD::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
                                CThostFtdcRspInfoField *pRspInfo,
                                int nRequestID, bool bIsLast) {
  const char *action = "query instruments rsp";
  if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
    ALERT("failed in %s, [%d]%s.\n", action, pRspInfo->ErrorID,
          CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str());
  } else {
    if (pInstrument) {
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
      instr.opt_type = pInstrument->OptionsType == THOST_FTDC_CP_CallOptions
                           ? OptionsType::OT_Call
                           : OptionsType::OT_Put;
      vec_instr.emplace_back(instr);

      // update in 'instrument_list' field of daily info
      json &j =
          DailyInfoMgr::j["instrument_list"][string(pInstrument->InstrumentID)];
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

      // init tick count of this instrument
      DailyInfoMgr::map_instr_cnt[string(pInstrument->InstrumentID)] = 0;
    }
    if (bIsLast) {
      status = 4;
      ENGLOG("succeed in %s.\n", action);
    }
  }
}
