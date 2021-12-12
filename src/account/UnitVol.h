#ifndef SRC_ACCOUNT_UNITVOL_H
#define SRC_ACCOUNT_UNITVOL_H

#include "json.hpp"
using json = nlohmann::json;

class UnitVol {
 public:
  json to_json();
  bool from_json(json& j);
  void onSwitchDay();

  void onBuyOpnNew(int vol);
  void onBuyOpnCxl(int vol);
  void onBuyOpnTrd(int vol);
  void onSellOpnNew(int vol);
  void onSellOpnCxl(int vol);
  void onSellOpnTrd(int vol);
  void onBuyClsYdNew(int vol);
  void onBuyClsYdCxl(int vol);
  void onBuyClsYdTrd(int vol);
  void onSellClsYdNew(int vol);
  void onSellClsYdCxl(int vol);
  void onSellClsYdTrd(int vol);
  void onBuyClsTdNew(int vol);
  void onBuyClsTdCxl(int vol);
  void onBuyClsTdTrd(int vol);
  void onSellClsTdNew(int vol);
  void onSellClsTdCxl(int vol);
  void onSellClsTdTrd(int vol);

  int pos_long_yd_ini = 0;      // initial long of yesterday
  int pos_long_yd = 0;          // current long of yesterday
  int pos_long_td = 0;          // today's long position
  int pos_long = 0;             // total long position
  int pos_long_yd_cls_ing = 0;  // close yesterday's long ongoing
  int pos_long_td_cls_ing = 0;  // close today's long ongoing
  int pos_long_td_opn_ing = 0;  // open today's long ongoing

  int pos_short_yd_ini = 0;      // initial short of yesterday
  int pos_short_yd = 0;          // current short of yesterday
  int pos_short_td = 0;          // today's short position
  int pos_short = 0;             // total short position
  int pos_short_yd_cls_ing = 0;  // close yesterday's short ongoing
  int pos_short_td_cls_ing = 0;  // close today's short onging
  int pos_short_td_opn_ing = 0;  // open today's short ongoing

  int pos_buy = 0;   // total buy of today
  int pos_sell = 0;  // total sell of today
};

#endif
