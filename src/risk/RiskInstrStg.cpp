#include "RiskInstrStg.h"

#include <string>

#include "ConfUtil.h"
#include "Logger.h"
#include "float.h"
using namespace std;

bool RiskInstrStg::init(const json& j_conf, const tInstrumentInfo* p_instr_info,
                        AccBase* ab) {
  string instr_str = string(p_instr_info->instr);
  string prd_str = string(p_instr_info->product);

  try {
    // allowed_price_tick: in the unit of one tick price
    if (!ConfUtil<double>::getValue(
            &allowed_price_margin_threshold,
            j_conf["/RiskForOrder/allowed_price_tick"_json_pointer], prd_str,
            instr_str) *
        p_instr_info->tick_price) {
      return false;
    }
  } catch (...) {
    ALERT("can't read j_conf.");
    return false;
  }

  // calculate the maximum price margin
  allowed_price_margin_threshold *= p_instr_info->tick_price;

  // other risk parameters
  if (!RiskInstr::init(j_conf, p_instr_info, ab)) {
    ALERT("can't init risk_instr.");
    return false;
  }

  return true;
}

/**
 * @brief Check whether the price exceeds the margin with respect to latest
 * ask/bid price in the market
 * @param[in] px Issued price of this order
 * @param[in] ask Latest market best sell price (i.e., ask1)
 * @param[in] bid Latest market best buy price (i.e., bid1)
 */
inline bool RiskInstrStg::IsPriceCorrect(double px, double ask, double bid) {
  if (ask != 0.0 && ask != DBL_MAX &&
      px > ask + allowed_price_margin_threshold) {
    return false;
  }
  if (bid != 0.0 && bid != DBL_MAX &&
      px < bid - allowed_price_margin_threshold) {
    return false;
  }

  return true;
}

/**
 * @brief Check the price first then other conventional checks
 */
int RiskInstrStg::check(int dir, int off, int vol, double px, double ask,
                        double bid, long nano) {
  if (!IsPriceCorrect(px, ask, bid)) {
    return -201;
  }

  return RiskInstr::check(dir, off, vol, nano);
}
