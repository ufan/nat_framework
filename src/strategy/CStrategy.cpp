/*
 * CStrategy.cpp
 *
 *  Created on: May 25, 2018
 *      Author: hongxu
 */

#include "CStrategy.h"

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

static void signal_handler(int signum) {
  if (getSharedData()) getSharedData()->is_exit = 1;
}

static void setup_signal_callback() {
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGINT, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGQUIT, signal_handler);
  std::signal(SIGKILL, signal_handler);
}

extern long cur_md_nano;

CStrategy::CStrategy() {}

CStrategy::~CStrategy() { release(); }

/**
 * @brief Init this strategy with a configuration file.
 * If environment variable 'STG_CFG' set, it will load configuration there.
 */
bool CStrategy::init(string config_file) {
  // 1. read config from environment variable if set
  char* p_cfg = getenv("STG_CFG");
  if (p_cfg) {
    return loadConfig(string(p_cfg));
  }

  // 2. load configuration file
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

bool CStrategy::initStr(string config_content) {
  char* p_cfg = getenv("STG_CFG");
  if (p_cfg) {
    return loadConfig(string(p_cfg));
  }
  return loadConfig(config_content);
}

/**
 * @details Subscribe quotes of one instrument or a collection of instruments.
 * The subscribed instruments will be recorded in a map collection and
 * registered to the risk management. A callback slot also exists, where user
 * can provide extra subscribing operations.
 */
bool CStrategy::subscribe(string instruments) {
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
      uint32_t hash = INSTR_NAME_TO_HASH(i.c_str());
      auto itr = instr_info_map_.find(hash);
      if (itr == instr_info_map_.end()) {
        tSubsInstrInfo& info = instr_info_map_[hash];
        info.base_info = CTradeBaseInfo::instr_info_[hash];
        sys_on_subscribe(i);  // callback slot
        if (!p_risk_stg_->regInstr(i.c_str())) {
          ALERT("account register instr %s err", i.c_str());
          return false;
        }
      }
    }
  }
  return true;
}

/**
 * @details Create a bar maker for an subscribed instrument. If the instrument
 * is not subscribed yet, also subscribe this instrument.
 */
bool CStrategy::subsBar(string instrument, long interval_min) {
  uint32_t hash = INSTR_NAME_TO_HASH(instrument.c_str());
  auto itr = instr_info_map_.find(hash);
  if (itr == instr_info_map_.end()) {
    if (!subscribe(instrument)) return false;
    itr = instr_info_map_.find(hash);
  }
  subs_bar_.emplace_back(&(itr->second.base_info), interval_min);
  return true;
}

/**
 * @brief Send new order to the td engine using td helper
 * @details Risk account will do some checking before and after sending
            User should maintain the returned track_id, since it's the
            unique identifier for  this order's track record and for
            canceling this order later.
 * @param[in] instr_hash The hash of instr
 * @param[in] price Description
 * @param[in] vol Description
 * @param[in] dir Description
 * @param[in] off Description
 * @param[in] acc_idx The idx of acc
 * @return track_id on success, < 0 on failure
 */
int CStrategy::sendOrder(uint32_t instr_hash, double price, int vol, int dir,
                         int off, int acc_idx) {
  if (*p_do_trade_) {  // check if trading is allowed

    // only do trade on subscribed instruments
    auto itr = instr_info_map_.find(instr_hash);
    if (itr != instr_info_map_.end()) {
      int ret = 0;

      // risk management before and after sending the order
      if (0 ==
          (ret = p_risk_stg_->check(dir, off, price, vol, &(itr->second)))) {
        ret = p_td_helper_->sendOrder(itr->second.base_info.instr, price, vol,
                                      dir, off, acc_idx, 0, instr_hash);
        p_risk_stg_->onNew(dir, off, price, vol, instr_hash,
                           itr->second.lst_tick.exch_time);
      }
      return ret;
    }
  }
  return -1;
}

/**
 * @brief Send delete order request to td engine using td helper
 * @param[in] order_id is the same index returned from sendOrder
 * @return 0 on sucess, -1 on failure
 */
int CStrategy::cancelOrder(int order_id) {
  return p_td_helper_->deleteOrder(order_id);
}

/**
 * @brief Load configuration and initialize this strategy instance
 * NOTE: the 'name' field is used to uniquely identify the following objects:
 *       1. this strategy instance
 *       2. the record in proc file
 *       3. the account/risk management of this strategy
 *       4. the td/md helper of this strategy
 */
bool CStrategy::loadConfig(string config_content) {
  // step 1: Config strategy control method: default is central strategy table
  // if 'use_shm_controller' is configured as 'False', the strategy is
  // ran as a dangling process.
  setStrategyConfig(config_content);
  setup_signal_callback();

  // step 2: read logger configuration and init it
  json j_conf = json::parse(config_content);
  initFastLogger(j_conf);

  // step 3: change the title of the running process to the strategy's name
  // This is a critical so that the process can be identified by its unique name
  if (!setProcTitle(j_conf["name"])) {
    ALERT("setProcTitle failed.");
    return false;
  }

  // step 4: init strategy shared area
  if (not initStrategyShared(j_conf["name"])) {
    ALERT("init strategy shared area err.");
    return false;
  }
  // node point to the this strategy and flags reset
  p_is_exit_ = &(getSharedData()->is_exit);
  p_do_trade_ = &(getSharedData()->do_trade);

  // setp 5: init account (risk management), NOTE: only use AccountDefault
  auto& acc_conf = j_conf["AccountDefault"];
  p_risk_stg_.reset(new RiskStg);
  string name = j_conf["name"];
  if (!p_risk_stg_->init(name.c_str(), acc_conf)) {
    ALERT("init account err.");
    return false;
  }

  // step 6: init md helper for fetching quotes from md engine
  auto& md_helper_conf = j_conf["MDHelper"];
  p_md_helper_.reset(
      CMDHelperFactory::create(md_helper_conf["type"], j_conf["name"]));
  if (!p_md_helper_ || !p_md_helper_->init(md_helper_conf)) {
    ALERT("init md helper err.");
    return false;
  }

  // step 7: init td helper for insert/delete order through td engine
  auto& td_helper_conf = j_conf["TDHelper"];
  p_td_helper_.reset(
      CTDHelperFactory::create(td_helper_conf["type"], j_conf["name"]));
  if (!p_td_helper_ || !p_td_helper_->init(td_helper_conf)) {
    ALERT("init td helper err.");
    return false;
  }

  // after the init td helper, the base info and order track records are
  // automatically reset to the latest status, do some accounts work
  if (!p_risk_stg_->load()) {
    ALERT("load last account err.");
    return false;
  }
  for (int i = 0; i < getOrderTrackCnt(); i++) {
    p_risk_stg_->onOrderTrack(&getOrderTrack(i));
  }

  // step 8: setup the callback for day switching
  setSwitchDayCallBack(
      bind(&CStrategy::pre_on_switch_day, this, placeholders::_1));

  // step 9: init a reader to system io based on user's config
  self_id_ = (int)HASH_STR(getStrategyName().c_str());
  if (j_conf.find("listen_sysio") != j_conf.end() &&
      j_conf["listen_sysio"].get<bool>() == true) {
    p_sys_io_reader_.reset(CSystemIO::instance().createReader());
  }

  return true;
}

/**
 * @brief Fetch new quotes (including corresponding timestamp) from md engine
 *        Exposed as python interface
 * @return the pointer to the new quote, timestamp is updated in cur_md_nano
 */
const UnitedMarketData* CStrategy::readMd() {
  return p_md_helper_->read(cur_md_nano);
}

inline void CStrategy::processTick(const UnitedMarketData* pmd) {
  if (pmd) {
    // only update to the latest quote for instruments subscribed
    auto itr = instr_info_map_.find(pmd->instr_hash);
    if (itr != instr_info_map_.end()) {
      tLastTick& lst = itr->second.lst_tick;
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
    }

    // Callback slots of td helper and this strategy
    p_td_helper_->on_tick(pmd);
    sys_on_tick(pmd);
    on_tick(pmd);

    // Bar maker callback
    for (auto& b : subs_bar_) {
      b.OnTick(pmd);
      Bar* pbar = nullptr;
      while (nullptr != (pbar = b.GetBar())) {
        on_bar(pbar);
      }
    }
  }
}

/**
 * @brief Process message from system io.
 *        User can configure whether listen on system io with flag
 *        'listen_sysio'
 */
inline void CStrategy::processSysIO() {
  if (!p_sys_io_reader_) return;
  uint32_t len;
  const tSysIOHead* p = (const tSysIOHead*)p_sys_io_reader_->read(len);
  if (p && p->to == self_id_) {
    switch (p->cmd) {
      case IO_USER_ADD_EXEC_ORDER: {  // TBU, overlap with td engine's same
                                      // function
        tIOUserAddExecOrder* pcmd = (tIOUserAddExecOrder*)p;
        p_risk_stg_->onNew(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol,
                           pcmd->instr_hash, CTimer::instance().getNano());
        if (auto pmod = p_risk_stg_->getModInstr(pcmd->instr_hash)) {
          pmod->onTrd(pcmd->dir, pcmd->off, pcmd->price, pcmd->vol, pcmd->vol,
                      pcmd->price, pcmd->vol);
        }
        break;
      }
      case IO_USER_CMD: {  // process user defined message
        on_msg(p->data, len);
        break;
      }
    }
  }
}

/**
 * @brief The main event loop
 */
void CStrategy::run() {
  ENGLOG("strategy process start running.");

  // Start callback
  sys_on_start();

  // Day switch callback
  pre_on_switch_day(CTradeBaseInfo::trading_day_);

  *p_is_exit_ = 0;
  while (*p_is_exit_ == 0) {
    // step 1: process tick data
    const UnitedMarketData* pmd = p_md_helper_->read(cur_md_nano);
    processTick(pmd);

    // step 2: process order rtn message
    const tRtnMsg* prtn = p_td_helper_->getRtn();
    if (prtn) {
      p_risk_stg_->onRtn(&getOrderTrack(prtn->local_id), prtn);
      sys_on_rtn(prtn);
      on_rtn(prtn);
    }

    // step 3: if not tick update and not rtn message, process timer and system
    // io
    if (!pmd and !prtn) {
      long nano = CTimer::instance().getNano();
      p_td_helper_->on_time(nano);
      sys_on_time(nano);
      on_time(nano);

      processSysIO();
    }
  }
  // Bar drawing finish
  barOnFinish();

  // Stop Callback
  sys_on_stop();

  ENGLOG("strategy process stopped.");
}

void CStrategy::stop() { *p_is_exit_ = 1; }

/**
 * @brief Release resources before the destruction of this strategy
 */
void CStrategy::release() {
  // remove helper clients from the td/md engines
  p_md_helper_.reset();
  p_td_helper_.reset();

  instr_info_map_.clear();  // clear the map of subscribted instrument

  if (trading_day_.size() &&
      trading_day_ >= acc_save_day_) {  // save the previous account info
    p_risk_stg_->save(acc_save_day_, true);
  }

  delStg(getStrategyName());  // delete this instance from strategy shared
                              // region, important in MT-environment? TBU
}

/**
 * @brief Reset the strategy on a new trading day
 */
void CStrategy::pre_on_switch_day(string day) {
  sys_on_switch_day(day);  // callback slot
  barOnFinish();

  p_md_helper_->clearMap();
  if (p_td_helper_) p_td_helper_->on_switch_day(day);
  instr_info_map_.clear();
  subs_bar_.clear();

  if (acc_save_day_.empty()) acc_save_day_ = day;
  if (p_risk_stg_ && day > acc_save_day_) {
    p_risk_stg_->save(acc_save_day_);
  }
  if (trading_day_.size())  // not the first day
  {
    p_risk_stg_->onSwitchDay();
  }

  on_switch_day(day);  // callback slot, may do some init work for new day

  trading_day_ = day;
  if (day > acc_save_day_) acc_save_day_ = day;
}

void CStrategy::barOnFinish() {
  for (auto& b : subs_bar_) {
    b.OnFinish();
    Bar* pbar = nullptr;
    while (nullptr != (pbar = b.GetBar())) {
      on_bar(pbar);
    }
  }
}
