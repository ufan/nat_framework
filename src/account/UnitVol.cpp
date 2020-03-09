#include "UnitVol.h"
#include "Logger.h"
#include "stdio.h"

json UnitVol::to_json()
{
	json j;
	j["pos_long_yd_ini"]				= pos_long_yd_ini;
	j["pos_long_yd_cls_ing"]			= pos_long_yd_cls_ing;
	j["pos_long_yd"]					= pos_long_yd;
	j["pos_long_td_opn_ing"]			= pos_long_td_opn_ing;
	j["pos_long_td_cls_ing"]			= pos_long_td_cls_ing;
	j["pos_long_td"]					= pos_long_td;
	j["pos_long"]						= pos_long;
	j["pos_short_yd_ini"]				= pos_short_yd_ini;
	j["pos_short_yd_cls_ing"]			= pos_short_yd_cls_ing;
	j["pos_short_yd"]					= pos_short_yd;
	j["pos_short_td_opn_ing"]			= pos_short_td_opn_ing;
	j["pos_short_td_cls_ing"]			= pos_short_td_cls_ing;
	j["pos_short_td"]					= pos_short_td;
	j["pos_short"]						= pos_short;
	j["pos_buy"]						= pos_buy;
	j["pos_sell"]						= pos_sell;

	return j;
}

bool UnitVol::from_json(json& j)
{
	try
	{
		pos_long_yd_ini			= j["pos_long"];
		pos_long_yd_cls_ing		= 0;
		pos_long_yd				= j["pos_long"];
		pos_long_td_opn_ing		= 0;
		pos_long_td_cls_ing		= 0;
		pos_long_td				= 0;
		pos_long				= j["pos_long"];
		pos_short_yd_ini		= j["pos_short"];
		pos_short_yd_cls_ing	= 0;
		pos_short_yd			= j["pos_short"];
		pos_short_td_opn_ing	= 0;
		pos_short_td_cls_ing	= 0;
		pos_short_td			= 0;
		pos_short				= j["pos_short"];
		pos_buy					= 0;
		pos_sell				= 0;
	}
	catch (...)
	{
		ALERT("can't parse from unit_vol json part.");
		return false;
	}
	
	return true;
}

void UnitVol::onSwitchDay()
{
	pos_long_yd_ini			= pos_long;
	pos_long_yd_cls_ing		= 0;
	pos_long_yd				= pos_long;
	pos_long_td_opn_ing		= 0;
	pos_long_td_cls_ing		= 0;
	pos_long_td				= 0;
	pos_short_yd_ini		= pos_short;
	pos_short_yd_cls_ing	= 0;
	pos_short_yd			= pos_short;
	pos_short_td_opn_ing	= 0;
	pos_short_td_cls_ing	= 0;
	pos_short_td			= 0;
	pos_buy					= 0;
	pos_sell				= 0;
}

void UnitVol::onBuyOpnNew(int vol)
{
	pos_long_td_opn_ing += vol;
}

void UnitVol::onBuyOpnCxl(int vol)
{
	pos_long_td_opn_ing -= vol;
}

void UnitVol::onBuyOpnTrd(int vol)
{
	pos_long_td_opn_ing -= vol;
	pos_long_td += vol;
	pos_long += vol;
	pos_buy += vol;
}

void UnitVol::onSellOpnNew(int vol)
{
	pos_short_td_opn_ing += vol;
}

void UnitVol::onSellOpnCxl(int vol)
{
	pos_short_td_opn_ing -= vol;
}

void UnitVol::onSellOpnTrd(int vol)
{
	pos_short_td_opn_ing -= vol;
	pos_short_td += vol;
	pos_short += vol;
	pos_sell += vol;
}

void UnitVol::onBuyClsYdNew(int vol)
{
	pos_short_yd_cls_ing += vol;
}

void UnitVol::onBuyClsYdCxl(int vol)
{
	pos_short_yd_cls_ing -= vol;
}

void UnitVol::onBuyClsYdTrd(int vol)
{
	pos_short_yd_cls_ing -= vol;
	pos_short_yd -= vol;
	pos_short -= vol;
	pos_buy += vol;
}

void UnitVol::onSellClsYdNew(int vol)
{
	pos_long_yd_cls_ing += vol;
}

void UnitVol::onSellClsYdCxl(int vol)
{
	pos_long_yd_cls_ing -= vol;
}

void UnitVol::onSellClsYdTrd(int vol)
{
	pos_long_yd_cls_ing -= vol;
	pos_long_yd -= vol;
	pos_long -= vol;
	pos_sell += vol;
}

void UnitVol::onBuyClsTdNew(int vol)
{
	pos_short_td_cls_ing += vol;
}

void UnitVol::onBuyClsTdCxl(int vol)
{
	pos_short_td_cls_ing -= vol;
}

void UnitVol::onBuyClsTdTrd(int vol)
{
	pos_short_td_cls_ing -= vol;
	pos_short_td -= vol;
	pos_short -= vol;
	pos_buy += vol;
}

void UnitVol::onSellClsTdNew(int vol)
{
	pos_long_td_cls_ing += vol;
}

void UnitVol::onSellClsTdCxl(int vol)
{
	pos_long_td_cls_ing -= vol;
}

void UnitVol::onSellClsTdTrd(int vol)
{
	pos_long_td_cls_ing -= vol;
	pos_long_td -= vol;
	pos_long -= vol;
	pos_sell += vol;
}
