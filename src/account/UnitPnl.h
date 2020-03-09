#ifndef SRC_ACCOUNT_UNITPNL_H
#define SRC_ACCOUNT_UNITPNL_H

#include "json.hpp"
using json = nlohmann::json;

class UnitPnl
{
public:
	json to_json();
	bool from_json(json& j);
	void onSwitchDay();
	
	double calcPosPnlChangeOnPxChange(double cur_px, double last_px, int pos_long_yd_ini, int pos_short_yd_ini, int vol_multiple);
	double calcTrdPnlChangeOnPxChange(double cur_px, double last_px, int pos_buy, int pos_sell, int vol_multiple);
	void onPosPnlChange(double delta);
	void onTrdPnlChange(double delta);
	
	double pos_pnl = 0.0;
	double trd_pnl = 0.0;
};

#endif