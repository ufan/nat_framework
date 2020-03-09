#ifndef SRC_ACCOUNT_UNITVOL_H
#define SRC_ACCOUNT_UNITVOL_H

#include "json.hpp"
using json = nlohmann::json;

class UnitVol
{
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

	int pos_long_yd_ini = 0;
	int pos_long_yd_cls_ing = 0;
	int pos_long_yd = 0;
	int pos_long_td_opn_ing = 0;
	int pos_long_td_cls_ing = 0;
	int pos_long_td = 0;
	int pos_long = 0;
	int pos_short_yd_ini = 0;
	int pos_short_yd_cls_ing = 0;
	int pos_short_yd = 0;
	int pos_short_td_opn_ing = 0;
	int pos_short_td_cls_ing = 0;
	int pos_short_td = 0;
	int pos_short = 0;
	int pos_buy = 0;
	int pos_sell = 0;
};

#endif