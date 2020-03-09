#include "RiskInstrStg.h"
#include "float.h"
#include "ConfUtil.h"
#include "Logger.h"
#include <string>
using namespace std;

bool RiskInstrStg::init(const json& j_conf, const tInstrumentInfo* p_instr_info, AccBase* ab)
{
	string instr_str = string(p_instr_info->instr);
	string prd_str = string(p_instr_info->product);

	try
	{
		// allowed_price_tick
		if (! ConfUtil<double>::getValue(&allowed_price_margin_threshold, j_conf["/RiskForOrder/allowed_price_tick"_json_pointer], prd_str, instr_str) * p_instr_info->tick_price)
		{
			return false;
		}
	}
	catch (...)
	{
		ALERT("can't read j_conf.");
		return false;
	}
	allowed_price_margin_threshold *= p_instr_info->tick_price;
	
	if (! RiskInstr::init(j_conf, p_instr_info, ab))
	{
		ALERT("can't init risk_instr.");
		return false;
	}
	
	return true;
}

// 检查订单价格是否正确
inline bool RiskInstrStg::IsPriceCorrect(double px, double ask, double bid)
{
	if (ask != 0.0 && ask != DBL_MAX && px > ask + allowed_price_margin_threshold) {
		return false;
	}
	if (bid != 0.0 && bid != DBL_MAX && px < bid - allowed_price_margin_threshold) {
		return false;
	}
	
	return true;
}

int RiskInstrStg::check(int dir, int off, int vol, double px, double ask, double bid, long nano)
{
	if (! IsPriceCorrect(px, ask, bid))
	{
		return -201;
	}
	
	return RiskInstr::check(dir, off, vol, nano);
}
