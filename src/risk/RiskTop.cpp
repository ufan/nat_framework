#include "RiskTop.h"

#include <glob.h>

#include <fstream>

#include "ATConstants.h"
#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "Logger.h"
#include "MurmurHash2.h"
#include "SysConf.h"
#include "string.h"
#include "utils.h"

bool RiskTop::init(const char* name, const json& j) {
  if (!acc_base_.init(name, j)) {
    ALERT("can't init acc_base_ %s.", name);
    return false;
  }

  if (!risk_acc_.init(j, &acc_base_)) {
    ALERT("can't init risk_acc_ %s.", name);
    return false;
  }

  j_conf_ = j;
  strncpy(stg_name_, name, sizeof(stg_name_));

  return true;
}

bool RiskTop::regInstr(const char* instr_name) {
  // reg instr into AccBase first
  if (!acc_base_.regInstr(instr_name)) {
    return false;
  }

  uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
  if (CTradeBaseInfo::instr_info_.find(instr_hash) ==
      CTradeBaseInfo::instr_info_.end()) {
    ALERT("can't find %s in CTradeBaseInfo.", instr_name);
    return false;
  }
  tInstrumentInfo* p_instr_info = &CTradeBaseInfo::instr_info_[instr_hash];
  uint32_t prd_hash = p_instr_info->product_hash;
  if (!isExistRiskPrd(prd_hash)) {
    if (!regRiskPrd(p_instr_info->product)) {
      ALERT("can't reg risk_prd %s.", p_instr_info->product);
      return false;
    }
  }

  if (!isExistRiskInstr(instr_hash)) {
    if (!regRiskInstr(instr_name)) {
      ALERT("can't reg risk_instr %s.", instr_name);
      return false;
    }
  }

  return true;
}

bool RiskTop::isExistRiskPrd(uint32_t prd_hash) {
  return map_risk_prd_.find(prd_hash) != map_risk_prd_.end();
}

bool RiskTop::isExistRiskInstr(uint32_t instr_hash) {
  return map_risk_instr_.find(instr_hash) != map_risk_instr_.end();
}

bool RiskTop::regRiskPrd(const char* prd_name) {
  uint32_t prd_hash = INSTR_NAME_TO_HASH(prd_name);
  if (!isExistRiskPrd(prd_hash)) {
    if (!map_risk_prd_[prd_hash].init(j_conf_, prd_name, &acc_base_)) {
      ALERT("can't init prd %s.", prd_name);
      return false;
    }
  }

  return true;
}

bool RiskTop::regRiskInstr(const char* instr_name) {
  uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
  if (!isExistRiskInstr(instr_hash)) {
    if (CTradeBaseInfo::instr_info_.find(instr_hash) ==
        CTradeBaseInfo::instr_info_.end()) {
      ALERT("can't find %s in CTradeBaseInfo.", instr_name);
      return false;
    }
    tInstrumentInfo* p_instr_info = &CTradeBaseInfo::instr_info_[instr_hash];
    if (!map_risk_instr_[instr_hash].init(j_conf_, p_instr_info, &acc_base_)) {
      ALERT("can't init instr %s.", instr_name);
      return false;
    }
  }

  return true;
}

int RiskTop::check(int dir, int off, double px, int vol, uint32_t instr_hash) {
  int ret = 0;
  auto itr = CTradeBaseInfo::instr_info_.find(instr_hash);
  if (itr == CTradeBaseInfo::instr_info_.end()) {
    ALERT("can't find hash %u in CTradeBaseInfo.", instr_hash);
    return false;
  }
  tInstrumentInfo* p_instr_info = &itr->second;

  auto itr2 = map_risk_instr_.find(p_instr_info->instr_hash);
  while (itr2 == map_risk_instr_.end()) {
    if (!regInstr(p_instr_info->instr)) {
      return -404;
    }
    itr2 = map_risk_instr_.find(p_instr_info->instr_hash);
  }

  // check by risk_instr_top
  // Engine-level order must be definite?
  RiskInstrTop& risk_instr_top = itr2->second;
  if (off == AT_CHAR_Auto || off == AT_CHAR_Close) {
    return -405;
  }

  ret = risk_instr_top.check(dir, off, px, vol, CTimer::instance().getNano());
  if (ret != 0) {
    return ret;
  }

  // check by risk_prd
  auto itr3 = map_risk_prd_.find(p_instr_info->product_hash);
  RiskPrd& risk_prd = itr3->second;
  ret = risk_prd.check(dir, off, vol);
  if (ret != 0) {
    return ret;
  }

  // check by risk_acc
  ret = risk_acc_.check(dir, off, px, vol, p_instr_info->vol_multiple);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

void RiskTop::onOrderTrack(const tOrderTrack* p_ord_trk) {
  acc_base_.onOrderTrack(p_ord_trk);
}

void RiskTop::onNew(int dir, int off, double px, int vol, uint32_t instr_hash,
                    long order_ref) {
  acc_base_.onNew(dir, off, px, vol, instr_hash);
  map_risk_instr_[instr_hash].onNew(dir, px, order_ref,
                                    CTimer::instance().getNano());
  //	LOG_DBG("[%ld]NEW, %s, %s, %.2lf, %d", order_ref, getDirString(dir),
  // getOffString(off), px, vol);
}

void RiskTop::onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg) {
  acc_base_.onRtn(p_ord_trk, p_rtn_msg);
  if (p_rtn_msg->msg_type & ODS(CANCELED)) {
    map_risk_instr_[p_ord_trk->instr_hash].onCxl(p_ord_trk);
    //		LOG_DBG("[%ld]CANCEL, %s, %s, %.2lf, %d", p_ord_trk->order_ref,
    // getDirString(p_rtn_msg->dir), getOffString(p_rtn_msg->off),
    // p_ord_trk->price, p_rtn_msg->vol);
  }
  if (p_rtn_msg->msg_type & ODS(EXECUTION)) {
    map_risk_instr_[p_ord_trk->instr_hash].onTrd(p_ord_trk);
    LOG_DBG("[%ld]EXECUTION, %s, %s, %.2lf, %d", p_ord_trk->order_ref,
            getDirString(p_rtn_msg->dir), getOffString(p_rtn_msg->off),
            p_rtn_msg->price, p_rtn_msg->vol);
  }
}

void RiskTop::onTickPx(uint32_t instr_hash, double tick_px) {
  acc_base_.onTickPx(instr_hash, tick_px);
}
