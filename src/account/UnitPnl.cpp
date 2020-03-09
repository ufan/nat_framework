#include "UnitPnl.h"
#include "Logger.h"
#include "stdio.h"

json UnitPnl::to_json()
{
	json j;
	j["pos_pnl"]					= pos_pnl;
	j["trd_pnl"]					= trd_pnl;

	return j;
}

bool UnitPnl::from_json(json& j)
{
	try
	{
		pos_pnl = 0.0;
		trd_pnl = 0.0;
	}
	catch (...)
	{
		ALERT("can't parse from unit_pnl json part.");
		return false;
	}
	
	return true;
}

void UnitPnl::onSwitchDay()
{
	pos_pnl = 0.0;
	trd_pnl = 0.0;
}

double UnitPnl::calcPosPnlChangeOnPxChange(double cur_px, double last_px, int pos_long_yd_ini, int pos_short_yd_ini, int vol_multiple)
{
	return (cur_px - last_px) * (pos_long_yd_ini - pos_short_yd_ini) * vol_multiple;
}

double UnitPnl::calcTrdPnlChangeOnPxChange(double cur_px, double last_px, int pos_buy, int pos_sell, int vol_multiple)
{
	return (cur_px - last_px) * (pos_buy - pos_sell) * vol_multiple;
}

void UnitPnl::onPosPnlChange(double delta)
{
	pos_pnl += delta;
}

void UnitPnl::onTrdPnlChange(double delta)
{
	trd_pnl += delta;
}
