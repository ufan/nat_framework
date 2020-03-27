#include "UnitPx.h"
#include "Logger.h"
#include "stdio.h"

json UnitPx::to_json()
{
	json j;
	j["stl_px_yd"]			= stl_px_yd;
	j["last_px"]			= last_px;
	j["avg_px_buy"]			= avg_px_buy;
	j["avg_px_sell"]		= avg_px_sell;

	return j;
}

bool UnitPx::from_json(json& j)
{
	try
	{
		stl_px_yd			= j["last_px"];
		last_px				= j["last_px"];
		avg_px_buy			= 0.0;
		avg_px_sell			= 0.0;
	}
	catch (...)
	{
		ALERT("can't parse from unit_px json part.");
		return false;
	}
	
	return true;
}

void UnitPx::onSwitchDay()
{
	stl_px_yd			= last_px;
	avg_px_buy			= 0.0;
	avg_px_sell			= 0.0;
}

double UnitPx::calcAvgPx(double amt, int pos, int vol_multiple)
{
	if (pos == 0 || vol_multiple == 0)
	{
		return 0.0;
	}
	else
	{
		return amt / pos / vol_multiple;
	}
}
