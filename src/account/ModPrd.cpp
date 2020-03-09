#include "ModPrd.h"
#include "Logger.h"
#include "string.h"

bool ModPrd::init(const char* name)
{
	strncpy(prd_name, name, sizeof(prd_name));
	
	return true;
}

json ModPrd::to_json()
{
	json j;
	j["unit_vol"]			= unit_vol.to_json();
	j["unit_amt"]			= unit_amt.to_json();
	j["unit_pnl"]			= unit_pnl.to_json();

	return j;
}

bool ModPrd::from_json(json& j)
{
	if (! unit_vol.from_json(j["unit_vol"]))
	{
		ALERT("can't parse from json node unit_vol.");
		return false;
	}

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

void ModPrd::onSwitchDay()
{
	unit_vol.onSwitchDay();
	unit_amt.onSwitchDay();
	unit_pnl.onSwitchDay();
}
