#ifndef SRC_BAR_BARMAKER_H_
#define SRC_BAR_BARMAKER_H_

#include <string>
#include <vector>

#include "ATStructure.h"
#include "Bar2.h"

using namespace std;

struct TimeSpan {
  long bgn_sec = 0;
  long end_sec = 0;
  bool is_auction = false;
};

class BarMaker {
 public:
  BarMaker(tInstrumentInfo* p_instr_info, int cycle_sec);

  void OnTick(const UnitedMarketData* p_umd);
  void OnFinish();
  Bar* GetBar();

  int getCycle() { return cycle_sec_; }
  string getInstr() { return instr; }

 private:
  void generateVecBar(tInstrumentInfo* p_instr_info, int cycle_sec);
  vector<TimeSpan> getTradingTimespan(string prd);
  bool isEobOverlap(long end_sec, vector<TimeSpan>& vec_ctts);

 private:
  // Meta info of this maker
  string instr;
  uint32_t instr_hash = 0;
  int cycle_sec_ = 0;

  vector<Bar2> vec_bar;  // collection of bars
  int bar_idx = -1;      // index of the current bar for updating
  int rtn_bar_idx = -1;

  UnitedMarketData last_umd;
  long* p_exch_nano = nullptr;
};

#endif
