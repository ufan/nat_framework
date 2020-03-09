#ifndef SRC_ACCOUNT_UNITAMT_H
#define SRC_ACCOUNT_UNITAMT_H

#include "UnitVol.h"
#include "json.hpp"
using json = nlohmann::json;

class UnitAmt
{
public:
	json to_json();
	bool from_json(json& j);
	void onSwitchDay();

	double calcAmtChangeOnPxChange(double cur_px, double last_px, int pos, int vol_multiple);
	double calcAmtChangeOnTrd(double trd_px, int trd_vol, int vol_multiple);
	double calcAmtIngChangeOnNewCxlTrd(double px, int vol, int vol_multiple);

	void onAmtLongYdSubIngChange(double amt);
	void onAmtLongTdAddIngChange(double amt);
	void onAmtLongTdSubIngChange(double amt);
	void onAmtLongChange(double amt);
	void onAmtShortYdSubIngChange(double amt);
	void onAmtShortTdAddIngChange(double amt);
	void onAmtShortTdSubIngChange(double amt);
	void onAmtShortChange(double amt);
	void onAmtBuyChange(double amt);
	void onAmtSellChange(double amt);

	double amt_long_yd_sub_ing = 0.0;
	double amt_long_td_add_ing = 0.0;
	double amt_long_td_sub_ing = 0.0;
	double amt_long = 0.0;
	double amt_short_yd_sub_ing = 0.0;
	double amt_short_td_add_ing = 0.0;
	double amt_short_td_sub_ing = 0.0;
	double amt_short = 0.0;
	double amt_buy = 0.0;
	double amt_sell = 0.0;
};

#endif