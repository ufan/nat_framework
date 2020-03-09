#include "ModAcc.h"
#include "Logger.h"
#include "string.h"

json ModAcc::to_json()
{
	json j;
	j["unfilled_order_cnt"] = unfilled_order_cnt;
	j["unit_amt"]			= unit_amt.to_json();
	j["unit_pnl"]			= unit_pnl.to_json();

	return j;
}

bool ModAcc::from_json(json& j)
{
	unfilled_order_cnt	= 0;

	if (! unit_amt.from_json(j["unit_amt"]))
	{
		ALERT("can't parse from json node unit_amt.");
		return false;
	}

	if (! unit_pnl.from_json(j["unit_pnl"]))
	{
		ALERT("can't parse from json node unit_pnl.");
		return false;
	}
	
	return true;
}

void ModAcc::onSwitchDay()
{
	unfilled_order_cnt = 0;
	unit_amt.onSwitchDay();
	unit_pnl.onSwitchDay();
}
