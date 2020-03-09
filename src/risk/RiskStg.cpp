#include "RiskStg.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "ATConstants.h"
#include "SysConf.h"
#include "Logger.h"
#include <glob.h>
#include "utils.h"
#include "string.h"
#include <fstream>

bool RiskStg::init(const char* name, const json& j)
{
	if (! acc_base_.init(name, j))
	{
		ALERT("can't init acc_base_ %s.", name);
		return false;
	}
	
	if (! risk_acc_.init(j, &acc_base_))
	{
		ALERT("can't init risk_acc_ %s.", name);
		return false;
	}
	
	j_conf_ = j;
	strncpy(stg_name_, name, sizeof(stg_name_));
	
	return true;
}

bool RiskStg::regInstr(const char* instr_name)
{
	// reg instr into AccBase first
	if (! acc_base_.regInstr(instr_name))
	{
		return false;
	}
	
	uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
	if (CTradeBaseInfo::instr_info_.find(instr_hash) == CTradeBaseInfo::instr_info_.end())
	{
		ALERT("can't find %s in CTradeBaseInfo.", instr_name);
		return false;
	}
	tInstrumentInfo* p_instr_info = &CTradeBaseInfo::instr_info_[instr_hash];
	uint32_t prd_hash = p_instr_info->product_hash;
	if (! isExistRiskPrd(prd_hash))
	{
		if (! regRiskPrd(p_instr_info->product))
		{
			ALERT("can't reg risk_prd %s.", p_instr_info->product);
			return false;
		}
	}
	
	if (! isExistRiskInstr(instr_hash))
	{
		if (! regRiskInstr(instr_name))
		{
			ALERT("can't reg risk_instr %s.", instr_name);
			return false;
		}
	}
	
	LOG_DBG("succ reg instr %s.", instr_name);
	return true;
}

bool RiskStg::isExistRiskPrd(uint32_t prd_hash)
{
	return map_risk_prd_.find(prd_hash) != map_risk_prd_.end();
}

bool RiskStg::isExistRiskInstr(uint32_t instr_hash)
{
	return map_risk_instr_.find(instr_hash) != map_risk_instr_.end();
}

bool RiskStg::regRiskPrd(const char* prd_name)
{
	uint32_t prd_hash = INSTR_NAME_TO_HASH(prd_name);
	if (! isExistRiskPrd(prd_hash))
	{
		if (! map_risk_prd_[prd_hash].init(j_conf_, prd_name, &acc_base_))
		{
			ALERT("can't init prd %s.", prd_name);
			return false;
		}
	}
	
	return true;
}

bool RiskStg::regRiskInstr(const char* instr_name)
{
	uint32_t instr_hash = INSTR_NAME_TO_HASH(instr_name);
	if (! isExistRiskInstr(instr_hash))
	{
		if (CTradeBaseInfo::instr_info_.find(instr_hash) == CTradeBaseInfo::instr_info_.end())
		{
			ALERT("can't find %s in CTradeBaseInfo.", instr_name);
			return false;
		}
		tInstrumentInfo* p_instr_info = &CTradeBaseInfo::instr_info_[instr_hash];
		if (! map_risk_instr_[instr_hash].init(j_conf_, p_instr_info, &acc_base_))
		{
			ALERT("can't init instr %s.", instr_name);
			return false;
		}
	}
	
	return true;
}

int RiskStg::check(int dir, int& off, double px, int vol, tSubsInstrInfo* p_subs_info)
{
	int ret = 0;
	tInstrumentInfo* p_instr_info = &p_subs_info->base_info;
	tLastTick* p_last_tick = &p_subs_info->lst_tick;
	auto itr = map_risk_instr_.find(p_instr_info->instr_hash);
	while (itr == map_risk_instr_.end())
	{
		if (! regInstr(p_instr_info->instr))
		{
			return -403;
		}
		itr = map_risk_instr_.find(p_instr_info->instr_hash);
	}
	
	// check by risk_instr_stg
	RiskInstrStg& risk_instr_stg = itr->second;
	if (off == AT_CHAR_Auto || off == AT_CHAR_Close)
	{
		ret = risk_instr_stg.parseAutoOffset(dir, off, vol);
		if (ret != 0)
		{
			return ret;
		}
	}
	
	ret = risk_instr_stg.check(dir, off, vol, px, p_last_tick->ask_px, p_last_tick->bid_px, p_last_tick->exch_time);
	if (ret != 0)
	{
		return ret;
	}
	
	// check by risk_prd
	auto itr2 = map_risk_prd_.find(p_instr_info->product_hash);
	RiskPrd& risk_prd = itr2->second;
	ret = risk_prd.check(dir, off, vol);
	if (ret != 0)
	{
		return ret;
	}
	
	// check by risk_acc
	ret = risk_acc_.check(dir, off, px, vol, p_instr_info->vol_multiple);
	if (ret != 0)
	{
		return ret;
	}

	return 0;
}

void RiskStg::onOrderTrack(const tOrderTrack* p_ord_trk)
{
	acc_base_.onOrderTrack(p_ord_trk);
}

void RiskStg::onNew(int dir, int off, double px, int vol, uint32_t instr_hash, long nano)
{
	acc_base_.onNew(dir, off, px, vol, instr_hash);
	map_risk_instr_[instr_hash].onNew(nano);
}

void RiskStg::onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg)
{
	acc_base_.onRtn(p_ord_trk, p_rtn_msg);
}

void RiskStg::onTickPx(uint32_t instr_hash, double tick_px)
{
	acc_base_.onTickPx(instr_hash, tick_px);
}

void RiskStg::onSwitchDay()
{
	acc_base_.onSwitchDay();
	
	vector<uint32_t> vec_erase;
	for (auto& kv : map_risk_instr_)
	{
		RiskInstrStg& instr = kv.second;
		if (instr.p_unit_vol->pos_long == 0 && instr.p_unit_vol->pos_short == 0)
		{
			vec_erase.push_back(kv.first);
		}
	}
	for (auto& hash : vec_erase)
	{
		map_risk_instr_.erase(hash);
	}
	
	vec_erase.clear();
	for (auto& kv : map_risk_prd_)
	{
		RiskPrd& prd = kv.second;
		if (prd.p_unit_vol->pos_long == 0 && prd.p_unit_vol->pos_short == 0)
		{
			vec_erase.push_back(kv.first);
		}
	}
	for (auto& hash : vec_erase)
	{
		map_risk_prd_.erase(hash);
	}
}

UnitAmt* RiskStg::getAccUnitAmt()
{
	return acc_base_.getAccUnitAmt();
}

UnitPnl* RiskStg::getAccUnitPnl()
{
	return acc_base_.getAccUnitPnl();
}

UnitVol* RiskStg::getInstrUnitVol(uint32_t instr_hash)
{
	return acc_base_.getInstrUnitVol(instr_hash);
}

UnitAmt* RiskStg::getInstrUnitAmt(uint32_t instr_hash)
{
	return acc_base_.getInstrUnitAmt(instr_hash);
}

UnitPx* RiskStg::getInstrUnitPx(uint32_t instr_hash)
{
	return acc_base_.getInstrUnitPx(instr_hash);
}

UnitPnl* RiskStg::getInstrUnitPnl(uint32_t instr_hash)
{
	return acc_base_.getInstrUnitPnl(instr_hash);
}

UnitVol* RiskStg::getPrdUnitVol(uint32_t prd_hash)
{
	return acc_base_.getPrdUnitVol(prd_hash);
}

UnitAmt* RiskStg::getPrdUnitAmt(uint32_t prd_hash)
{
	return acc_base_.getPrdUnitAmt(prd_hash);
}

UnitPnl* RiskStg::getPrdUnitPnl(uint32_t prd_hash)
{
	return acc_base_.getPrdUnitPnl(prd_hash);
}
