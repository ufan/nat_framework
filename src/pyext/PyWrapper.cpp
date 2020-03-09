/*
 * PyWrapper.cpp
 *
 *  Created on: May 30, 2018
 *      Author: hongxu
 */

#include "stddef.h"
#include "PyWrapper.h"


bp::list getInstrumentInfo()
{
	bp::list res;
	for(auto &kv : CTradeBaseInfo::instr_info_)
	{
		res.append(kv.second);
	}
	return res;
}

bp::dict tick2Dict(const UnitedMarketData* pmd)
{
	bp::dict res;

	res["instr_hash"] = pmd->instr_hash;
	res["last_px"] = pmd->last_px;
	res["cum_vol"] = pmd->cum_vol;
	res["cum_turnover"] = pmd->cum_turnover;
	res["avg_px"] = pmd->avg_px;
	res["ask_px"] = pmd->ask_px;
	res["bid_px"] = pmd->bid_px;
	res["ask_vol"] = pmd->ask_vol;
	res["bid_vol"] = pmd->bid_vol;
	res["ask_px2"] = pmd->ask_px2;
	res["bid_px2"] = pmd->bid_px2;
	res["ask_vol2"] = pmd->ask_vol2;
	res["bid_vol2"] = pmd->bid_vol2;
	res["ask_px3"] = pmd->ask_px3;
	res["bid_px3"] = pmd->bid_px3;
	res["ask_vol3"] = pmd->ask_vol3;
	res["bid_vol3"] = pmd->bid_vol3;
	res["ask_px4"] = pmd->ask_px4;
	res["bid_px4"] = pmd->bid_px4;
	res["ask_vol4"] = pmd->ask_vol4;
	res["bid_vol4"] = pmd->bid_vol4;
	res["ask_px5"] = pmd->ask_px5;
	res["bid_px5"] = pmd->bid_px5;
	res["ask_vol5"] = pmd->ask_vol5;
	res["bid_vol5"] = pmd->bid_vol5;
	res["open_interest"] = pmd->open_interest;
	res["exch_time"] = pmd->exch_time;
	res["instr_str"] = pmd->instr_str;

	return res;
}
