#ifndef SRC_ACCOUNT_MODINSTR_H
#define SRC_ACCOUNT_MODINSTR_H

#include <string>

#include "ModAcc.h"
#include "ModPrd.h"
#include "UnitAmt.h"
#include "UnitPnl.h"
#include "UnitPx.h"
#include "UnitVol.h"
using namespace std;

class ModInstr {
 public:
  bool init(const char* name, int vm);

  json to_json();
  bool from_json(json& j);
  void onSwitchDay();

  void onTickPx(double tick_px);
  void onNew(int dir, int off, double px, int vol);
  void onCxl(int dir, int off, double px, int cxl_vol);
  void onTrd(int dir, int off, double px, int vol, int vol_traded,
             double trd_px, int trd_vol);
  string getInstrName() { return instr_name; }

  UnitVol unit_vol;  // volume
  UnitAmt unit_amt;  // amount = px * vol * vol_multiple
  UnitPx unit_px;    // price
  UnitPnl unit_pnl;  // position net loss?

  ModPrd* p_prd = nullptr;  // product summary
  ModAcc* p_acc = nullptr;  // account summary

  // meta info
  char instr_name[32] = {0};
  int vol_multiple = 0;
};

#endif
