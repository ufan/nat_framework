/*
 * CTradeBaseInfo.cpp
 *
 *  Created on: May 28, 2018
 *      Author: hongxu
 */

#include "CTradeBaseInfo.h"

#include "string.h"

bool CTradeBaseInfo::is_init_ = false;
string CTradeBaseInfo::trading_day_;
unordered_map<uint32_t, tInstrumentInfo> CTradeBaseInfo::instr_info_;
switchday_fn CTradeBaseInfo::switch_day_cb_ = nullptr;

// Set to an existing trade base, used in MDEngine
void CTradeBaseInfo::set(const tIOTDBaseInfo *p) {
  trading_day_ = p->trading_day;

  instr_info_.clear();
  for (int i = 0; i < p->instr_cnt; ++i) {
    instr_info_[p->instr[i].instr_hash] = p->instr[i];
  }
  is_init_ = true;
}

// Update the trade base in day-switching.
// Nothing is done on the same trading day.
bool CTradeBaseInfo::update(const tIOTDBaseInfo *p) {
  if (trading_day_ != p->trading_day) {
    set(p);
    if (switch_day_cb_) {
      switch_day_cb_(trading_day_);
    }
    return true;
  }
  return false;
}

// Compose the datagram using the trade base information
string CTradeBaseInfo::toSysIOStruct(int to, int source, int back_word) {
  uint32_t size = sizeof(tSysIOHead) + sizeof(tIOTDBaseInfo) +
                  instr_info_.size() * sizeof(tInstrumentInfo);
  string data(size, 0);
  tSysIOHead *rtn_head = (tSysIOHead *)data.data();
  rtn_head->cmd = IO_TD_RSP_BASE_INFO;
  rtn_head->to = to;
  rtn_head->source = source;
  rtn_head->back_word = back_word;

  tIOTDBaseInfo *p = (tIOTDBaseInfo *)rtn_head->data;
  strncpy(p->trading_day, trading_day_.c_str(), sizeof(p->trading_day) - 1);
  p->instr_cnt = instr_info_.size();

  int i = 0;
  for (auto &kv : instr_info_) {
    memcpy(p->instr + i, &(kv.second), sizeof(tInstrumentInfo));
    i++;
  }
  return data;
}

// Get the list of instruments matching 'product' id
vector<tInstrumentInfo> CTradeBaseInfo::getInstrInProduct(string product) {
  bool is_all = product == "all" || product == "All";
  vector<tInstrumentInfo> res;
  for (auto &kv : instr_info_) {
    if (product == kv.second.product || is_all) {
      res.push_back(kv.second);
    }
  }
  return res;
}

/**
 * @brief Return a set of instrument names
 * @param[in] product element could be one of follows:
 *            1. 'all' or 'All': for all instruments in the market
 *            2. product name: for all instruments of this product
 *            3. instrument name
 * @return Description
 */
std::set<string> CTradeBaseInfo::productToInstrSet(
    const vector<string> &product) {
  std::set<string> res;
  for (auto &i : product) {
    bool is_all = i == "all" || i == "All";
    for (auto &kv : instr_info_) {
      auto &info = kv.second;
      if (i == info.instr || i == info.product || is_all) {
        res.insert(info.instr);
      }
    }
  }
  return res;
}

std::set<string> CTradeBaseInfo::productOrInstrumentToInstrSet(
    const vector<string> &product) {
  std::set<string> res;
  for (auto &i : product) {
    bool is_all = i == "all" || i == "All";
    for (auto &kv : instr_info_) {
      auto &info = kv.second;
      if (i == info.instr || i == info.product || is_all) {
        res.insert(info.instr);
      } else {
        res.insert(i);
      }
    }
  }
  return res;
}

void CTradeBaseInfo::addInstrInfo(tInstrumentInfo info) {
  instr_info_[info.instr_hash] = info;
}

void CTradeBaseInfo::callOnSwitchDayCb() { switch_day_cb_(trading_day_); }

int exchangeStr2int(string exch) {
  if (exch == "SSE")
    return EXCHANGEID_SSE;
  else if (exch == "SZE")
    return EXCHANGEID_SZE;
  else if (exch == "CFFEX")
    return EXCHANGEID_CFFEX;
  else if (exch == "SHFE")
    return EXCHANGEID_SHFE;
  else if (exch == "DCE")
    return EXCHANGEID_DCE;
  else if (exch == "CZCE")
    return EXCHANGEID_CZCE;
  else if (exch == "INE")
    return EXCHANGEID_INE;
  else if (exch == "SGE")
    return EXCHANGEID_SGE;
  return -1;
}

const char *exchangeint2str(int exch) {
  switch (exch) {
    case EXCHANGEID_SSE:
      return "SSE";
    case EXCHANGEID_SZE:
      return "SZE";
    case EXCHANGEID_CFFEX:
      return "CFFEX";
    case EXCHANGEID_SHFE:
      return "SHFE";
    case EXCHANGEID_DCE:
      return "DCE";
    case EXCHANGEID_CZCE:
      return "CZCE";
    case EXCHANGEID_INE:
      return "INE";
    case EXCHANGEID_SGE:
      return "SGE";
  }
  return nullptr;
}
