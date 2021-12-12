#include "BarMaker.h"

#include "ATConstants.h"
#include "BarHelper.h"
#include "CTradeBaseInfo.h"

int getDailyCycle() { return AT_INT_DAILY_CYCLE; }

int getSessionCycle() { return AT_INT_SESSION_CYCLE; }

BarMaker::BarMaker(tInstrumentInfo* p_instr_info, int cycle_sec) {
  // 1) generate the bars of this instrument with FRQ = cycle_sec
  generateVecBar(p_instr_info, cycle_sec);

  // 2) setup meta info of this maker
  cycle_sec_ = cycle_sec;
  instr = p_instr_info->instr;
  instr_hash = p_instr_info->instr_hash;

  // 3) start time of each exchange? TBU
  switch (p_instr_info->exch) {
    case EXCHANGEID_CZCE:
      p_exch_nano = &BarHelper::CZCE_nano;
      break;
    case EXCHANGEID_DCE:
      p_exch_nano = &BarHelper::DCE_nano;
      break;
    case EXCHANGEID_SHFE:
      p_exch_nano = &BarHelper::SHFE_nano;
      break;
    case EXCHANGEID_CFFEX:
      p_exch_nano = &BarHelper::CFFEX_nano;
      break;
    default:
      p_exch_nano = &BarHelper::OTHER_nano;
      break;
  }
}

void BarMaker::generateVecBar(tInstrumentInfo* p_instr_info, int cycle_sec) {
  Bar2 bar, bar2;
  bar.cycle_sec = cycle_sec;
  bar.instr_hash = p_instr_info->instr_hash;
  strncpy(bar.instr_str, p_instr_info->instr, sizeof(bar.instr_str));
  bar2.cycle_sec = cycle_sec;
  bar2.instr_hash = p_instr_info->instr_hash;
  strncpy(bar2.instr_str, p_instr_info->instr, sizeof(bar2.instr_str));

  // Get the trading periods for this product
  vector<TimeSpan> vec_ctts = getTradingTimespan(p_instr_info->product);
  vec_bar.clear();
  long leave = 0;

  if (cycle_sec == AT_INT_DAILY_CYCLE) {  // one bar for a whole day
    for (auto it = vec_ctts.begin(); it < vec_ctts.end(); ++it) {
      if (it == vec_ctts.begin()) {
        bar.bob = it->bgn_sec;
        bar.adjust_bob = 0;
      }

      // TBU
      bar.eob = it->end_sec;
      bar.adjust_eob = bar.eob + 1;
      if (p_instr_info->exch == EXCHANGEID_CZCE) {
        bar.settle_sec = bar.eob - 1;
      } else {
        bar.settle_sec = bar.eob;
      }
    }
    bar.is_auction = false;
    vec_bar.push_back(bar);
  } else if (cycle_sec == AT_INT_SESSION_CYCLE) {  // one bar for each period
    for (auto it = vec_ctts.begin(); it < vec_ctts.end(); ++it) {
      bar.bob = it->bgn_sec;
      if (it == vec_ctts.begin()) {
        bar.adjust_bob = 0;
      } else {
        bar.adjust_bob = bar.bob;
      }

      bar.eob = it->end_sec;
      if (it->is_auction) {
        bar.adjust_eob = bar.eob;
      } else {
        bar.adjust_eob = bar.eob + 1;
      }

      if (p_instr_info->exch == EXCHANGEID_CZCE) {
        bar.settle_sec = bar.eob - 1;
      } else {
        bar.settle_sec = bar.eob;
      }
      bar.is_auction = it->is_auction;
      vec_bar.push_back(bar);
    }
  } else {
    for (auto it = vec_ctts.begin(); it < vec_ctts.end(); ++it) {
      if (it->is_auction) {  // only one bar in the auction period
        bar2.bob = it->bgn_sec;
        bar2.eob = it->end_sec;
        bar2.is_auction = true;
        vec_bar.push_back(bar2);
      } else {  // continuous trading period
        if (leave == 0) {
          // previous cycle end overlaps begin of time span
          // thus, this is a full cycle in this period
          bar.bob = it->bgn_sec;
          bar.eob = bar.bob + cycle_sec;
        } else {  // previous cycle has remaining time
          // compose the previous cycle using time in this period
          bar.eob = it->bgn_sec + leave;
        }

        while (bar.bob < it->end_sec && bar.eob <= it->end_sec) {
          bar.is_auction = false;
          vec_bar.push_back(bar);
          bar.bob = bar.eob;
          bar.eob = bar.eob + cycle_sec;
        }

        if (bar.bob != it->end_sec) {
          leave = bar.eob - it->end_sec;
        }
      }
    }

    if (leave > 0) {              // last cycle is not full in the last period
      bar.eob = bar.eob - leave;  // end the last cycle at the end of the period
      bar.is_auction = false;
      vec_bar.push_back(bar);
    }

    // adjust the bob and eob, but why? TBU
    for (auto it = vec_bar.begin(); it != vec_bar.end(); ++it) {
      if (it->is_auction) {
        if (it == vec_bar.begin()) {
          it->adjust_bob = 0;  // TBU
        } else {
          it->adjust_bob = it->bob - 2 * 3600;  // TBU
        }

        it->adjust_eob = it->eob;
        it->settle_sec = it->adjust_eob;
      } else {
        it->adjust_bob = it->bob;

        if (isEobOverlap(it->eob, vec_ctts)) {
          it->adjust_eob = it->eob + 1;
          //					if (p_instr_info->exch ==
          // EXCHANGEID_CZCE)
          //					{
          //						it->settle_sec = it->eob
          //- 1;
          //					}
          //					else
          //					{
          it->settle_sec = it->eob;
          //					}
        } else {
          it->adjust_eob = it->eob;
        }
      }
    }
  }
  //	int cnt = 0;
  //	for (auto it = vec_bar.begin(); it < vec_bar.end(); ++it, ++cnt)
  //	{
  //		printf("vec_bar[%d/%lu] %d %02ld:%02ld:%02ld(%02ld:%02ld:%02ld)
  //%02ld:%02ld:%02ld(%02ld:%02ld:%02ld), settle: %02ld:%02ld:%02ld\n"
  //, cnt,
  // vec_bar.size(), it->is_auction 			, (18+it->bob/3600)%24,
  // it->bob%3600/60,
  // it->bob%60 			, (18+it->adjust_bob/3600)%24,
  // it->adjust_bob%3600/60, it->adjust_bob%60 			,
  // (18+it->eob/3600)%24, it->eob%3600/60, it->eob%60 			,
  //(18+it->adjust_eob/3600)%24, it->adjust_eob%3600/60, it->adjust_eob%60
  //, (18+it->settle_sec/3600)%24, it->settle_sec%3600/60, it->settle_sec%60
  //			);
  //	}
}

void BarMaker::OnTick(const UnitedMarketData* p_umd) {
  if (p_umd->instr_hash != instr_hash) {
    return;
  }

  if (p_umd->exch_time > *p_exch_nano) {
    *p_exch_nano = p_umd->exch_time;
  }

  if (bar_idx == -1) {
    bar_idx = 0;
  }
  if (bar_idx >= vec_bar.size()) {
    return;
  }
  Bar2* p_bar = &vec_bar[bar_idx];

  long base_sec_interval = BarHelper::getBaseIntervalFromNano(p_umd->exch_time);
  int pos = p_bar->isInCurBar(base_sec_interval);
  while (true) {
    if (pos == 0) {
      // in cur bar
      if (p_bar->isNeedUpdateOpen(p_umd)) {
        p_bar->updateOpen(p_umd);
      }

      if (p_bar->isNeedUpdatePre()) {
        //				printf("in_cur_bar, need_update_pre,
        //%s\n", p_bar->instr_str);
        p_bar->updatePre(p_umd);
      }

      if (p_bar->isNeedUpdateOhlc(p_umd)) {
        p_bar->updateBar(p_umd);
      }

      if (p_bar->isNeedSettle(p_umd)) {
        //				printf("need_settle, %d, %lu\n",
        // bar_idx, vec_bar.size());
        p_bar->settle(p_umd);
        if (bar_idx < vec_bar.size()) {
          ++bar_idx;
          if (bar_idx < vec_bar.size()) {
            p_bar = &vec_bar[bar_idx];
            p_bar->updatePre(p_umd);
          }
        }
        //				printf("need_settle end, %d, %lu\n",
        // bar_idx, vec_bar.size());
      }
      break;
    } else if (pos == 1) {
      // in next bar
      //			if (p_bar->is_auction == false)
      //			{
      p_bar->settle(p_umd);
      //			}
      if (bar_idx < vec_bar.size() - 1) {
        ++bar_idx;
        if (bar_idx < vec_bar.size()) {
          p_bar = &vec_bar[bar_idx];
          if (last_umd.instr_hash != 0) {
            p_bar->updatePre(&last_umd);
          }
          pos = p_bar->isInCurBar(base_sec_interval);
        } else {
          break;
        }
      } else {
        break;
      }
    } else {
      // in pre bar
      //			printf("in pre bar, %d, %lu, %ld, %s\n",
      // bar_idx, vec_bar.size(), p_umd->exch_time, p_umd->instr_str);
      if (bar_idx < vec_bar.size()) {
        p_bar->updatePre(p_umd);
      }
      break;
    }
  }
  last_umd = *p_umd;
}

void BarMaker::OnFinish() {
  if (bar_idx == -1) {
    bar_idx = 0;
  }

  while (bar_idx < vec_bar.size()) {
    Bar2* p_bar = &vec_bar[bar_idx];
    p_bar->settle(&last_umd);
    ++bar_idx;
  }
}

bool BarMaker::isEobOverlap(long end_sec, vector<TimeSpan>& vec_ctts) {
  for (TimeSpan& ts : vec_ctts) {
    if (ts.end_sec == end_sec) {
      return true;
    }
  }
  return false;
}

vector<TimeSpan> BarMaker::getTradingTimespan(string prd) {
  vector<TimeSpan> vec_ctts;
  TimeSpan ts;
  if (prd == "IC" || prd == "IF" || prd == "IH") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 29 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 30 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "T" || prd == "TF" || prd == "TS") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 14 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 15 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 15 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "AP" || prd == "JR" || prd == "LR" || prd == "PM" ||
             prd == "RI" || prd == "RS" || prd == "SF" || prd == "SM" ||
             prd == "WH" || prd == "bb" || prd == "c" || prd == "cs" ||
             prd == "fb" || prd == "jd" || prd == "l" || prd == "pp" ||
             prd == "v" || prd == "fu" || prd == "wr") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(8 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "CF" || prd == "CY" || prd == "FG" || prd == "MA" ||
             prd == "OI" || prd == "RM" || prd == "SR" || prd == "TA" ||
             prd == "ZC" || prd == "a" || prd == "b" || prd == "i" ||
             prd == "j" || prd == "jm" || prd == "m" || prd == "p" ||
             prd == "y") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(20 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(23 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(8 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "bu" || prd == "hc" || prd == "rb" || prd == "ru") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(20 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(23 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(8 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "al" || prd == "cu" || prd == "ni" || prd == "pb" ||
             prd == "sn" || prd == "zn") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(20 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(1 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(8 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else if (prd == "ag" || prd == "au" || prd == "sc") {
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(20 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(21 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(2 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(8 * 3600 + 59 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.is_auction = true;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(9 * 3600 + 0 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 15 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(10 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(11 * 3600 + 30 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
    ts.bgn_sec = BarHelper::getBaseIntervalFromBjtSec(13 * 3600 + 30 * 60);
    ts.end_sec = BarHelper::getBaseIntervalFromBjtSec(15 * 3600 + 0 * 60);
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  } else {
    ts.bgn_sec = 0;
    ts.end_sec = 24 * 3600 - 1;
    ts.is_auction = false;
    vec_ctts.push_back(ts);
  }
  return vec_ctts;
}

// TBU
Bar* BarMaker::GetBar() {
  //	printf("%d, %d, %d, %ld\n", cycle_sec_, rtn_bar_idx, bar_idx,
  // last_umd.exch_time);
  //
  //	if (bar_idx != -1 && bar_idx < vec_bar.size())
  //	{
  //		Bar2* p_bar = &vec_bar[bar_idx];
  //		long exch_sec =
  // BarHelper::getBaseIntervalFromNano(*p_exch_nano); 		if (exch_sec -
  // p_bar->eob > DELAY_SEC && !p_bar->is_auction)
  //		{
  //			if (last_umd.instr_hash != 0)
  //			{
  //				p_bar->settle(&last_umd);
  //			}
  //			if (bar_idx < vec_bar.size())
  //			{
  //				++bar_idx;
  //				if (bar_idx < vec_bar.size())
  //				{
  //					p_bar = &vec_bar[bar_idx];
  //					if (last_umd.instr_hash != 0)
  //					{
  //						p_bar->updatePre(&last_umd);
  //					}
  //				}
  //			}
  //		}
  //	}

  Bar* p_rtn_bar = nullptr;
  while (rtn_bar_idx < bar_idx) {
    if (rtn_bar_idx != -1) {
      p_rtn_bar = &vec_bar[rtn_bar_idx];
      if (p_rtn_bar->open == 0.0) {
        p_rtn_bar = nullptr;
      }
    }
    ++rtn_bar_idx;
  }

  return p_rtn_bar;
}
