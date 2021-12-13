#ifndef SRC_ACCOUNT_UNITPX_H
#define SRC_ACCOUNT_UNITPX_H

#include "UnitAmt.h"
#include "UnitVol.h"
#include "json.hpp"
using json = nlohmann::json;

class UnitPx {
 public:
  json to_json();
  bool from_json(json& j);
  void onSwitchDay();

  // avg_px = amt/pos/vol_multiple
  double calcAvgPx(double amt, int pos, int vol_multiple);

  double stl_px_yd = 0.0;    // settle price yesterday
  double last_px = 0.0;      // last trade price
  double avg_px_buy = 0.0;   // average buy price
  double avg_px_sell = 0.0;  // average sell price
};

#endif
