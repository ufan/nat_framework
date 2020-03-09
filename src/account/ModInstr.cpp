#include "ModInstr.h"
#include "ATConstants.h"
#include "Logger.h"
#include "string.h"
#include "stdio.h"

bool ModInstr::init(const char* name, int vm)
{
	if (vm < 0)
	{
		ALERT("vol_multiple should not < 0 !");
		return false;
	}
	
	strncpy(instr_name, name, sizeof(instr_name));
	vol_multiple = vm;
	
	return true;
}

json ModInstr::to_json()
{
	json j;
	j["unit_vol"]			= unit_vol.to_json();
	j["unit_amt"]			= unit_amt.to_json();
	j["unit_px"]			= unit_px.to_json();
	j["unit_pnl"]			= unit_pnl.to_json();

	return j;
}

bool ModInstr::from_json(json& j)
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

	if (! unit_px.from_json(j["unit_px"]))
	{
		ALERT("can't parse from json node unit_px.");
		return false;
	}

	if (! unit_pnl.from_json(j["unit_pnl"]))
	{
		ALERT("can't parse from json node unit_pnl.");
		return false;
	}
	
	return true;
}

void ModInstr::onSwitchDay()
{
	unit_vol.onSwitchDay();
	unit_amt.onSwitchDay();
	unit_px.onSwitchDay();
	unit_pnl.onSwitchDay();
}

void ModInstr::onTickPx(double tick_px)
{
	double delta = 0.0;
	delta = unit_amt.calcAmtChangeOnPxChange(tick_px, unit_px.last_px, unit_vol.pos_long, vol_multiple);
//	LOG_DBG("amt_long_chg=%.2lf", delta);
	if (0.0 != delta)
	{
		unit_amt.onAmtLongChange(delta);
		p_prd->unit_amt.onAmtLongChange(delta);
		p_acc->unit_amt.onAmtLongChange(delta);
	}
	delta = unit_amt.calcAmtChangeOnPxChange(tick_px, unit_px.last_px, unit_vol.pos_short, vol_multiple);
//	LOG_DBG("amt_short_chg=%.2lf", delta);
	if (0.0 != delta)
	{
		unit_amt.onAmtShortChange(delta);
		p_prd->unit_amt.onAmtShortChange(delta);
		p_acc->unit_amt.onAmtShortChange(delta);
	}
	delta = unit_pnl.calcPosPnlChangeOnPxChange(tick_px, unit_px.last_px, unit_vol.pos_long_yd_ini, unit_vol.pos_short_yd_ini, vol_multiple);
//	LOG_DBG("pos_pnl_chg=%.2lf", delta);
	if (0.0 != delta)
	{
		unit_pnl.onPosPnlChange(delta);
		p_prd->unit_pnl.onPosPnlChange(delta);
		p_acc->unit_pnl.onPosPnlChange(delta);
	}
	delta = unit_pnl.calcTrdPnlChangeOnPxChange(tick_px, unit_px.last_px, unit_vol.pos_buy, unit_vol.pos_sell, vol_multiple);
//	LOG_DBG("trd_pnl_chg=%.2lf", delta);
	if (0.0 != delta)
	{
		unit_pnl.onTrdPnlChange(delta);
		p_prd->unit_pnl.onTrdPnlChange(delta);
		p_acc->unit_pnl.onTrdPnlChange(delta);
	}
	
	unit_px.last_px = tick_px;
}

void ModInstr::onNew(int dir, int off, double px, int vol)
{
	double amt_ing_chg = unit_amt.calcAmtIngChangeOnNewCxlTrd(px, vol, vol_multiple);
	++p_acc->unfilled_order_cnt;
	switch (dir)
	{
		case AT_CHAR_Buy:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onBuyOpnNew(vol);
					p_prd->unit_vol.onBuyOpnNew(vol);
					unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onBuyClsYdNew(vol);
					p_prd->unit_vol.onBuyClsYdNew(vol);
					unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onBuyClsTdNew(vol);
					p_prd->unit_vol.onBuyClsTdNew(vol);
					unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			break;
		case AT_CHAR_Sell:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onSellOpnNew(vol);
					p_prd->unit_vol.onSellOpnNew(vol);
					unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onSellClsYdNew(vol);
					p_prd->unit_vol.onSellClsYdNew(vol);
					unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onSellClsTdNew(vol);
					p_prd->unit_vol.onSellClsTdNew(vol);
					unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			break;
		default:
			ALERT("unknown dir=%d.", dir);
			break;
	}
}

void ModInstr::onCxl(int dir, int off, double px, int cxl_vol)
{
	double amt_ing_chg = -1 * unit_amt.calcAmtIngChangeOnNewCxlTrd(px, cxl_vol, vol_multiple);
	--p_acc->unfilled_order_cnt;
	switch (dir)
	{
		case AT_CHAR_Buy:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onBuyOpnCxl(cxl_vol);
					p_prd->unit_vol.onBuyOpnCxl(cxl_vol);
					unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onBuyClsYdCxl(cxl_vol);
					p_prd->unit_vol.onBuyClsYdCxl(cxl_vol);
					unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onBuyClsTdCxl(cxl_vol);
					p_prd->unit_vol.onBuyClsTdCxl(cxl_vol);
					unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			break;
		case AT_CHAR_Sell:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onSellOpnCxl(cxl_vol);
					p_prd->unit_vol.onSellOpnCxl(cxl_vol);
					unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onSellClsYdCxl(cxl_vol);
					p_prd->unit_vol.onSellClsYdCxl(cxl_vol);
					unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onSellClsTdCxl(cxl_vol);
					p_prd->unit_vol.onSellClsTdCxl(cxl_vol);
					unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			break;
		default:
			ALERT("unknown dir=%d.", dir);
			break;
	}
}

void ModInstr::onTrd(int dir, int off, double px, int vol, int vol_traded, double trd_px, int trd_vol)
{
	onTickPx(trd_px);
	
	double amt_ing_chg = -1 * unit_amt.calcAmtIngChangeOnNewCxlTrd(px, trd_vol, vol_multiple);
	double amt_chg = unit_amt.calcAmtChangeOnTrd(trd_px, trd_vol, vol_multiple);
	if (vol == vol_traded)
	{
		--p_acc->unfilled_order_cnt;
	}
	switch (dir)
	{
		case AT_CHAR_Buy:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onBuyOpnTrd(trd_vol);
					p_prd->unit_vol.onBuyOpnTrd(trd_vol);
					unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdAddIngChange(amt_ing_chg);
//					LOG_DBG("amt_long_chg=%.2lf", amt_chg);
					unit_amt.onAmtLongChange(amt_chg);
					p_prd->unit_amt.onAmtLongChange(amt_chg);
					p_acc->unit_amt.onAmtLongChange(amt_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onBuyClsYdTrd(trd_vol);
					p_prd->unit_vol.onBuyClsYdTrd(trd_vol);
					unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortYdSubIngChange(amt_ing_chg);
//					LOG_DBG("amt_short_chg=%.2lf", -1 * amt_chg);
					unit_amt.onAmtShortChange(-1 * amt_chg);
					p_prd->unit_amt.onAmtShortChange(-1 * amt_chg);
					p_acc->unit_amt.onAmtShortChange(-1 * amt_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onBuyClsTdTrd(trd_vol);
					p_prd->unit_vol.onBuyClsTdTrd(trd_vol);
					unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdSubIngChange(amt_ing_chg);
//					LOG_DBG("amt_short_chg=%.2lf", -1 * amt_chg);
					unit_amt.onAmtShortChange(-1 * amt_chg);
					p_prd->unit_amt.onAmtShortChange(-1 * amt_chg);
					p_acc->unit_amt.onAmtShortChange(-1 * amt_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			unit_amt.onAmtBuyChange(amt_chg);
			unit_px.avg_px_buy = unit_px.calcAvgPx(unit_amt.amt_buy, unit_vol.pos_buy, vol_multiple);
			break;
		case AT_CHAR_Sell:
			switch (off)
			{
				case AT_CHAR_Open:
					unit_vol.onSellOpnTrd(trd_vol);
					p_prd->unit_vol.onSellOpnTrd(trd_vol);
					unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtShortTdAddIngChange(amt_ing_chg);
//					LOG_DBG("amt_short_chg=%.2lf", amt_chg);
					unit_amt.onAmtShortChange(amt_chg);
					p_prd->unit_amt.onAmtShortChange(amt_chg);
					p_acc->unit_amt.onAmtShortChange(amt_chg);
					break;
				case AT_CHAR_CloseYesterday:
					unit_vol.onSellClsYdTrd(trd_vol);
					p_prd->unit_vol.onSellClsYdTrd(trd_vol);
					unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongYdSubIngChange(amt_ing_chg);
//					LOG_DBG("amt_long_chg=%.2lf", -1 * amt_chg);
					unit_amt.onAmtLongChange(-1 * amt_chg);
					p_prd->unit_amt.onAmtLongChange(-1 * amt_chg);
					p_acc->unit_amt.onAmtLongChange(-1 * amt_chg);
					break;
				case AT_CHAR_CloseToday:
					unit_vol.onSellClsTdTrd(trd_vol);
					p_prd->unit_vol.onSellClsTdTrd(trd_vol);
					unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_prd->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
					p_acc->unit_amt.onAmtLongTdSubIngChange(amt_ing_chg);
//					LOG_DBG("amt_long_chg=%.2lf", -1 * amt_chg);
					unit_amt.onAmtLongChange(-1 * amt_chg);
					p_prd->unit_amt.onAmtLongChange(-1 * amt_chg);
					p_acc->unit_amt.onAmtLongChange(-1 * amt_chg);
					break;
				default:
					ALERT("unknown off=%d.", off);
					break;
			}
			unit_amt.onAmtSellChange(amt_chg);
			unit_px.avg_px_sell = unit_px.calcAvgPx(unit_amt.amt_sell, unit_vol.pos_sell, vol_multiple);
			break;
		default:
			ALERT("unknown dir=%d.", dir);
			break;
	}
}
