/*
 * CStrategyProcess.cpp
 *
 *  Created on: 2018年5月9日
 *      Author: sky
 */

#include "CStrategyProcess.h"

#include <stdio.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <csignal>
#include <fstream>

#include "CMDHelperFactory.h"
#include "CTDHelperFactory.h"
#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "StrategyShared.h"
#include "SysConf.h"

IMDHelper* CStrategyProcess::p_md_helper_ = nullptr;
ITDHelper* CStrategyProcess::p_td_helper_ = nullptr;
vector<CStrategyBase*> CStrategyProcess::strategy_list_;
umap<uint32_t, CStrategyProcess::tSubscribedInstrInfo>
    CStrategyProcess::instr_info_map_;
volatile uint32_t* CStrategyProcess::p_is_exit_ = nullptr;
volatile uint32_t* CStrategyProcess::p_do_trade_ = nullptr;
vector<pair<BarMaker, vector<CStrategyBase*>>> CStrategyProcess::subs_bar_info_;
string CStrategyProcess::trading_day_;
string CStrategyProcess::acc_save_day_;

static void signal_handler(int signum) { CStrategyProcess::stop(); }

static void setup_signal_callback() {
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGINT, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGQUIT, signal_handler);
  std::signal(SIGKILL, signal_handler);
}

long cur_md_nano = -1;
long getCurMdTime()  // td_helper may use this function
{
  return cur_md_nano;
}

/**
 * @brief Initialize the running environment using a combination of default and
 * user-customized configuration. Same as CStrategy::init.
 */
bool CStrategyProcess::init(string config_file) {
  char* p_cfg = getenv("STG_CFG");
  if (p_cfg) {
    return loadConfig(string(p_cfg));
  }

  ifstream in(config_file);
  if (!in) {
    fprintf(stderr, "read config file %s err.\n", config_file.c_str());
    return false;
  }
  string content((std::istreambuf_iterator<char>(in)),
                 std::istreambuf_iterator<char>());
  in.close();
  return loadConfig(content);
}

bool CStrategyProcess::initStr(string config_content) {
  char* p_cfg = getenv("STG_CFG");
  if (p_cfg) {
    return loadConfig(string(p_cfg));
  }
  return loadConfig(config_content);
}

/**
 * @brief Load configuration and initialization of all components
 * @details Same order of steps as CStrategy::loadConfig, except that the
 * account/risk management part is now moved to CStrategyBase and system io is
 * removed completely.
 * NOTE: the 'name' and its corresponding hash_id is now an unique
 * identification of this CStrategyProcess instance, which may be composed of
 * multiple base strategies. Each strategy is now identified by the index of its
 * position in the strategy list of this CStrategyProcess instance.
 */
bool CStrategyProcess::loadConfig(string config_content) {
  setStrategyConfig(config_content);
  setup_signal_callback();

  json j_conf = json::parse(config_content);
  initFastLogger(j_conf);

  if (!setProcTitle(j_conf["name"])) {
    ALERT("setProcTitle failed.");
    return false;
  }

  // init shared area
  if (not initStrategyShared(j_conf["name"])) {
    ALERT("init strategy shared area err.");
    return false;
  }
  p_is_exit_ = &(getSharedData()->is_exit);
  p_do_trade_ = &(getSharedData()->do_trade);

  // init md
  auto& md_helper_conf = j_conf["MDHelper"];
  p_md_helper_ =
      CMDHelperFactory::create(md_helper_conf["type"], j_conf["name"]);
  if (p_md_helper_ == nullptr || !p_md_helper_->init(md_helper_conf)) {
    ALERT("init md helper err.");
    return false;
  }

  // init td
  auto& td_helper_conf = j_conf["TDHelper"];
  p_td_helper_ =
      CTDHelperFactory::create(td_helper_conf["type"], j_conf["name"]);
  if (p_td_helper_ == nullptr || !p_td_helper_->init(td_helper_conf)) {
    ALERT("init td helper err.");
    return false;
  }

  setSwitchDayCallBack(
      bind(&CStrategyProcess::pre_on_switch_day, placeholders::_1));
  return true;
}

/**
 * @brief Add and init a new base strategy and its risk account
 * @details Add a base strategy to the strategy list and assign the self_id_ to
 *          identify this base strategy.
 * @param[in] stg pointer to the new base strategy
 * @param[in] profile_id index of the accounts in 'Account' field of the
 *            configuration file. NOTE: it's not the same as the self_id_ ofthe
 *            base strategy, which is the index of position in the strategy
 *            list.
 * @return    index id of the position of the strategy in the list, i.e.,
 *            self_id
 */
int CStrategyProcess::add(CStrategyBase* stg, int profile_id) {
  // step1: add to the list
  strategy_list_.push_back(stg);
  int id = strategy_list_.size() - 1;
  stg->self_id_ = id;

  // step2: select account profile
  json j_conf = json::parse(getStrategyConfig());
  auto acc_conf = j_conf["Account"];
  if (profile_id >= acc_conf.size())
    acc_conf = j_conf["AccountDefault"];
  else
    acc_conf = acc_conf[profile_id];

  // step3: create and init the risk account. TBU
  stg->p_risk_stg_.reset(new RiskStg);
  string stg_name = j_conf["name"].get<string>() +
                    string(SUB_ACCOUNT_FILE_SUFFIX) + to_string(profile_id);
  // 3.1) init associate trade account and risk account
  if (!stg->p_risk_stg_->init(stg_name.c_str(), acc_conf)) {
    ALERT("init strategy %d account failed.", profile_id);
    return -1;
  }

  // 3.2) load trade account into the state of previous trading
  if (!stg->p_risk_stg_->load()) {
    ALERT("load last account err.");
    return false;
  }

  // 3.3) update trade account based on existing order track
  for (int i = 0; i < getOrderTrackCnt(); i++) {
    tOrderTrack& ot = getOrderTrack(i);
    if (ot.stg_id == stg->self_id_) {
      stg->p_risk_stg_->onOrderTrack(&ot);
    }
  }
  // stg->on_switch_day(CTradeBaseInfo::trading_day_);
  return id;
}

template <class T>
static bool isInVector(vector<T>& v, T val) {
  for (auto& i : v) {
    if (i == val) return true;
  }
  return false;
}

/**
 * @brief Subscribe to new instruments for a specific base strategy
 * @details The following relation are established: 1) md engine will subscribe
 * the quotes for the new instruments; 2) push new instrument to the list of
 * subscribed instruments; 3) handling the 1-to-N mapping between instrument to
 * strategies.
 * @param[in] stg The base strategy which will subscribe to the new instruments
 * @param[in] instruments Maybe the name of a single or multiple
 * products/instruments, separated by ',' or ';' or 'All' or 'all'.
 */
bool CStrategyProcess::subscribe(CStrategyBase* stg, string instruments) {
  if (not instruments.empty()) {
    vector<string> destination;
    boost::split(destination, instruments, boost::is_any_of(",; "),
                 boost::token_compress_on);
    if (!p_md_helper_->subscribe(destination)) {
      ALERT("subscribe failed.");
      return false;
    }

    auto instr_set = CTradeBaseInfo::productToInstrSet(destination);
    if (instr_set.empty()) {
      ALERT("no target instrument found in base info list.");
      return false;
    }
    for (auto& i : instr_set) {
      bool isNew = false;
      uint32_t hash = INSTR_NAME_TO_HASH(i.c_str());
      auto itr = instr_info_map_.find(hash);
      if (itr == instr_info_map_.end()) {
        tSubscribedInstrInfo& info = instr_info_map_[hash];
        info.instr_info.base_info = CTradeBaseInfo::instr_info_[hash];
        info.stgs.push_back(stg);
        isNew = true;
      } else {
        auto& stgs = itr->second.stgs;
        if (!isInVector(stgs, stg)) {
          stgs.push_back(stg);
          isNew = true;
        }
      }
      if (isNew && !stg->p_risk_stg_->regInstr(i.c_str())) {
        ALERT("account register instr %s err", i.c_str());
        return false;
      }
    }
  }
  return true;
}

/**
 * @brief Subscribe new instruments and create bar makers as well
 * @details Subscribe to new instruments if not subscribed yet, then create the
 * bar maker for each new instrument. It will establish the 1-to-N mapping
 * between bar marker list and base strategies.
 * @param[in] stg same as subscribe()
 * @param[in] instrument same as subscribe()
 * @param[in] interval_min time inverval used in the bar graph. Bar maker is
 * identified by the subscribed instrument and the time interval.
 */
bool CStrategyProcess::subsBar(CStrategyBase* stg, string instrument,
                               long interval_min) {
  if (!subscribe(stg, instrument)) return false;
  for (auto& i : subs_bar_info_) {
    if (i.first.getCycle() == interval_min &&
        i.first.getInstr() == instrument) {
      i.second.push_back(stg);
      return true;
    }
  }
  uint32_t hash = INSTR_STR_TO_HASH(instrument);
  tInstrumentInfo& info = CTradeBaseInfo::instr_info_[hash];
  subs_bar_info_.emplace_back(BarMaker(&info, interval_min),
                              vector<CStrategyBase*>{stg});
  return true;
}

inline void CStrategyProcess::on_tick(const UnitedMarketData* pmd) {
  // td helper may process all quotes subscribed by td engine
  p_td_helper_->on_tick(pmd);

  // this strategy process instance only needs to process instruments subcribed
  auto itr = instr_info_map_.find(pmd->instr_hash);
  if (itr != instr_info_map_.end()) {
    tLastTick& lst = itr->second.instr_info.lst_tick;
    lst.ask_px = pmd->ask_px;
    lst.ask_vol = pmd->ask_vol;
    lst.bid_px = pmd->bid_px;
    lst.bid_vol = pmd->bid_vol;
    lst.ask_px2 = pmd->ask_px2;
    lst.ask_vol2 = pmd->ask_vol2;
    lst.bid_px2 = pmd->bid_px2;
    lst.bid_vol2 = pmd->bid_vol2;
    lst.ask_px3 = pmd->ask_px3;
    lst.ask_vol3 = pmd->ask_vol3;
    lst.bid_px3 = pmd->bid_px3;
    lst.bid_vol3 = pmd->bid_vol3;
    lst.ask_px4 = pmd->ask_px4;
    lst.ask_vol4 = pmd->ask_vol4;
    lst.bid_px4 = pmd->bid_px4;
    lst.bid_vol4 = pmd->bid_vol4;
    lst.ask_px5 = pmd->ask_px5;
    lst.ask_vol5 = pmd->ask_vol5;
    lst.bid_px5 = pmd->bid_px5;
    lst.bid_vol5 = pmd->bid_vol5;
    lst.last_price = pmd->last_px;
    lst.cum_vol = pmd->cum_vol;
    lst.exch_time = pmd->exch_time;

    // each base strategy subscribed to this instrument
    auto& stgs = itr->second.stgs;
    for (auto& s : stgs) {
      s->on_tick(pmd);
    }

    // each bar maker subscribed to this instrument
    for (auto& b : subs_bar_info_) {
      b.first.OnTick(pmd);
      Bar* pbar = nullptr;
      while (nullptr != (pbar = b.first.GetBar())) {
        for (auto s : b.second) s->on_bar(pbar);
      }
    }
  }
}

inline void CStrategyProcess::on_time(long nano) {
  p_td_helper_->on_time(nano);
  for (auto& s : strategy_list_) {
    s->on_time(nano);
  }
}

inline void CStrategyProcess::on_rtn(const tRtnMsg* prtn) {
  CStrategyBase* stg = strategy_list_[p_td_helper_->getStgIdFromRtnMsg(prtn)];
  stg->p_risk_stg_->onRtn(&getOrderTrack(prtn->local_id), prtn);
  stg->on_rtn(prtn);
}

void CStrategyProcess::run() {
  ENGLOG("strategy process start running.");

  CStrategyProcess::pre_on_switch_day(CTradeBaseInfo::trading_day_);

  *p_is_exit_ = 0;
  while (*p_is_exit_ == 0) {
    const UnitedMarketData* pmd = p_md_helper_->read(cur_md_nano);
    if (pmd) on_tick(pmd);

    const tRtnMsg* prtn = p_td_helper_->getRtn();
    if (prtn) on_rtn(prtn);

    if (!pmd and !prtn) on_time(CTimer::instance().getNano());
  }
  barOnFinish();

  ENGLOG("strategy process stopped.");
}

void CStrategyProcess::stop() { *p_is_exit_ = 1; }

void CStrategyProcess::release() {
  if (p_md_helper_) {
    delete p_md_helper_;
    p_md_helper_ = nullptr;
  }

  if (p_td_helper_) {
    delete p_td_helper_;
    p_td_helper_ = nullptr;
  }

  if (trading_day_.size() && trading_day_ >= acc_save_day_) {
    for (auto& s : strategy_list_) {
      if (s->p_risk_stg_) {
        s->p_risk_stg_->save(acc_save_day_, true);
      }
    }
  }
  strategy_list_.clear();
  instr_info_map_.clear();

  delStg(getStrategyName());
}

/**
 * @brief Operations on a new trading day, same as CStrategy
 */
void CStrategyProcess::pre_on_switch_day(string day) {
  barOnFinish();

  p_md_helper_->clearMap();
  if (p_td_helper_) p_td_helper_->on_switch_day(day);
  instr_info_map_.clear();
  subs_bar_info_.clear();

  if (acc_save_day_.empty()) acc_save_day_ = day;
  if (day > acc_save_day_) {
    for (auto& s : strategy_list_) {
      s->p_risk_stg_->save(acc_save_day_);
      if (trading_day_.size())  // not the first day
      {
        s->p_risk_stg_->onSwitchDay();
      }
      s->on_switch_day(day);
    }
  }
  trading_day_ = day;
  if (day > acc_save_day_) acc_save_day_ = day;
}

void CStrategyProcess::barOnFinish() {
  for (auto& b : subs_bar_info_) {
    b.first.OnFinish();
    Bar* pbar = nullptr;
    while (nullptr != (pbar = b.first.GetBar())) {
      for (auto s : b.second) s->on_bar(pbar);
    }
  }
}
