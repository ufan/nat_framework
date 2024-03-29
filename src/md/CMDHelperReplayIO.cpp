/*
 * CMDHelperReplayIO.cpp
 *
 *  Created on: 2018年5月14日
 *      Author: hongxu
 */

#include "CMDHelperReplayIO.h"

#include <set>

#include "CTimer.h"
#include "CTradeBaseInfo.h"
#include "IOCommon.h"
#include "StrategyShared.h"
#include "compiler.h"

int g_replay_md_fake_wait_count = 0;

bool CMDHelperReplayIO::init(const json& j_conf) {
  return _init(j_conf["path"], j_conf["start"], j_conf["end"]);
}

bool CMDHelperReplayIO::_init(string path, string start, string end) {
  long start_nano = parseTime(start.c_str(), "%Y%m%d-%H:%M:%S");
  end_nano_ = parseTime(end.c_str(), "%Y%m%d-%H:%M:%S");
  reader_.init(path, start_nano);
  return scan();
}

const UnitedMarketData* CMDHelperReplayIO::read(long& md_nano) {
  // mandatory latency
  if (g_replay_md_fake_wait_count-- > 0)
    return nullptr;
  else
    g_replay_md_fake_wait_count = 50;

  while (not has_finish_) {
    // get the current frame addr
    const tFrameHead* p_frame = reader_.getCurFrame();

    // reach end of replay
    if (unlikely(!p_frame || p_frame->nano > end_nano_)) {
      has_finish_ = true;
      getSharedData()->is_exit = 1;
      ENGLOG("replay data finished.");
      return nullptr;
    }

    if (*(int*)(p_frame->buf) == IO_MARKET_DATA) {
      const UnitedMarketData* pmd =
          &(((const tIOMarketData*)(p_frame->buf))->market_data);

      // only push the subscribed quotes
      if (filter(pmd)) {
        reader_.passFrame();
        continue;
      }

      // point to next frame
      reader_.passFrame();

      // align the clock time to simulate history timeflow
      md_nano = p_frame->nano;
      if (not first_tick_) {
        if (md_nano > CTimer::instance().getNano())
          CTimer::instance().setTime(md_nano);
      } else {
        first_tick_ = false;
        CTimer::instance().setTime(md_nano);
      }
      return pmd;
    } else if (*(int*)(p_frame->buf) ==
               IO_TD_RSP_BASE_INFO)  // new base info update
    {
      CTradeBaseInfo::update((tIOTDBaseInfo*)((tSysIOHead*)p_frame->buf)->data);
    }

    // point to next frame
    reader_.passFrame();
  }

  return nullptr;
}

void CMDHelperReplayIO::release() {
  IMDHelper::release();
  reader_.unload();
}

// scan the pages to get the first base info record and init trade base repo
bool CMDHelperReplayIO::scan() {
  uint32_t len;
  tIOMarketData* p = nullptr;
  CTradeBaseInfo::is_init_ = false;
  CTradeBaseInfo::trading_day_.clear();
  while (nullptr != (p = (tIOMarketData*)reader_.read(len))) {
    if (getIOFrameHead(p)->nano > end_nano_) {
      break;
    }
    if (p->cmd == IO_TD_RSP_BASE_INFO)  // base info update
    {
      CTradeBaseInfo::update((tIOTDBaseInfo*)((tSysIOHead*)p)->data);
      break;
    }
  }
  return CTradeBaseInfo::is_init_;
}

bool CMDHelperReplayIO::doSubscribe(const vector<string>& instr) {
  for (const auto& i : instr) {
    uint32_t hash = INSTR_STR_TO_HASH(i);
    if (CTradeBaseInfo::instr_info_.count(hash) == 0) {
      return false;
    }
  }
  return true;
}

// interface provided for python wrapper
const UnitedMarketData* CMDHelperReplayIO::Next() {
  uint32_t len;
  tIOMarketData* p = nullptr;
  while (nullptr != (p = (tIOMarketData*)reader_.read(len))) {
    if (getIOFrameHead(p)->nano > end_nano_) {
      break;
    }
    if (p->cmd == IO_MARKET_DATA) {
      if (not filter(&(p->market_data))) {
        return &(p->market_data);
      }
    } else if (p->cmd == IO_TD_RSP_BASE_INFO)  // base info update
    {
      CTradeBaseInfo::update((tIOTDBaseInfo*)((tSysIOHead*)p)->data);
    }
  }
  return nullptr;
}

vector<string> CMDHelperReplayIO::getEngineSubscribedInstrument() {
  vector<string> res;
  for (auto& kv : CTradeBaseInfo::instr_info_) {
    res.push_back(kv.second.instr);
  }
  return res;
}
