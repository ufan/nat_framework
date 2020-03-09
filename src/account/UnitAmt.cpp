#include "UnitAmt.h"
#include "Logger.h"
#include "stdio.h"

json UnitAmt::to_json()
{
	json j;
	j["amt_long_yd_sub_ing"]	= amt_long_yd_sub_ing;
	j["amt_long_td_add_ing"]	= amt_long_td_add_ing;
	j["amt_long_td_sub_ing"]	= amt_long_td_sub_ing;
	j["amt_long"]				= amt_long;
	j["amt_short_yd_sub_ing"]	= amt_short_yd_sub_ing;
	j["amt_short_td_add_ing"]	= amt_short_td_add_ing;
	j["amt_short_td_sub_ing"]	= amt_short_td_sub_ing;
	j["amt_short"]				= amt_short;
	j["amt_buy"]				= amt_buy;
	j["amt_sell"]				= amt_sell;

	return j;
}

bool UnitAmt::from_json(json& j)
{
	try
	{
		amt_long_yd_sub_ing		= 0.0;
		amt_long_td_add_ing		= 0.0;
		amt_long_td_sub_ing		= 0.0;
		amt_long				= j["amt_long"];
		amt_short_yd_sub_ing	= 0.0;
		amt_short_td_add_ing	= 0.0;
		amt_short_td_sub_ing	= 0.0;
		amt_short				= j["amt_short"];
		amt_buy					= 0.0;
		amt_sell				= 0.0;
	}
	catch (...)
	{
		ALERT("can't parse from unit_amt json part.");
		return false;
	}
	
	return true;
}

void UnitAmt::onSwitchDay()
{
	amt_long_yd_sub_ing		= 0.0;
	amt_long_td_add_ing		= 0.0;
	amt_long_td_sub_ing		= 0.0;
	amt_short_yd_sub_ing	= 0.0;
	amt_short_td_add_ing	= 0.0;
	amt_short_td_sub_ing	= 0.0;
	amt_buy					= 0.0;
	amt_sell				= 0.0;
}

double UnitAmt::calcAmtChangeOnPxChange(double cur_px, double last_px, int pos, int vol_multiple)
{
	return (cur_px - last_px) * pos * vol_multiple;
}

double UnitAmt::calcAmtChangeOnTrd(double trd_px, int trd_vol, int vol_multiple)
{
	return trd_px * trd_vol * vol_multiple;
}

double UnitAmt::calcAmtIngChangeOnNewCxlTrd(double px, int vol, int vol_multiple)
{
	return px * vol * vol_multiple;
}

void UnitAmt::onAmtLongYdSubIngChange(double amt)
{
	amt_long_yd_sub_ing += amt;
}

void UnitAmt::onAmtLongTdAddIngChange(double amt)
{
	amt_long_td_add_ing += amt;
}

void UnitAmt::onAmtLongTdSubIngChange(double amt)
{
	amt_long_td_sub_ing += amt;
}

void UnitAmt::onAmtLongChange(double amt)
{
	amt_long += amt;
}

void UnitAmt::onAmtShortYdSubIngChange(double amt)
{
	amt_short_yd_sub_ing += amt;
}

void UnitAmt::onAmtShortTdAddIngChange(double amt)
{
	amt_short_td_add_ing += amt;
}

void UnitAmt::onAmtShortTdSubIngChange(double amt)
{
	amt_short_td_sub_ing += amt;
}

void UnitAmt::onAmtShortChange(double amt)
{
	amt_short += amt;
}

void UnitAmt::onAmtBuyChange(double amt)
{
	amt_buy += amt;
}

void UnitAmt::onAmtSellChange(double amt)
{
	amt_sell += amt;
}
