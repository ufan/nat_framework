/*
 * CExecuteStrategy.cpp
 *
 *  Created on: 2018年12月17日
 *      Author: hongxu
 */

#include "CExecuteStrategy.h"
#include "StrategyShared.h"
#include "Logger.h"

CExecuteStrategy::CExecuteStrategy()
{

}

CExecuteStrategy::~CExecuteStrategy()
{

}

int CExecuteStrategy::execution(uint32_t instr_hash, double priceup,
		double pricedown, int placedsize, int direction, double duration)
{
	auto itr = loss_model_.find(instr_hash);
	if(itr != loss_model_.end())
	{
		return itr->second->OnExecution(priceup, pricedown, placedsize, direction, duration);
	}
	return false;
}

void CExecuteStrategy::cancelExec(uint32_t instr_hash, int execid)
{
	auto itr = loss_model_.find(instr_hash);
	if(itr != loss_model_.end())
	{
		return itr->second->cancelExecution(execid);
	}
}

void CExecuteStrategy::sys_on_tick(const UnitedMarketData* md)
{
	auto itr = loss_model_.find(md->instr_hash);
	if(itr != loss_model_.end())
	{
		itr->second->sys_on_tick(md);
	}
}

void CExecuteStrategy::sys_on_time(long nano)
{
	for(auto &kv : loss_model_)
	{
		kv.second->sys_on_time(nano);
	}
}

void CExecuteStrategy::sys_on_rtn(const tRtnMsg* rtn)
{
	auto itr = loss_model_.find(rtn->instr_hash);
	if(itr != loss_model_.end())
	{
		itr->second->sys_on_rtn(rtn);
	}
}

void CExecuteStrategy::sys_on_switch_day(string day)
{
	for(auto &kv : loss_model_)
	{
		kv.second->sys_on_switch_day(day);
	}
}

void CExecuteStrategy::sys_on_subscribe(string instr)
{
	uint32_t hash = INSTR_STR_TO_HASH(instr);
	auto itr = loss_model_.find(hash);
	if(itr == loss_model_.end())
	{
		unique_ptr<CLossModel> m(new CLossModel(this));
		if(not m->init(instr, getStrategyConfig()))
		{
			ALERT("init execution model for %s failed.", instr.c_str());
			return;
		}
		loss_model_[hash] = std::move(m);
	}
}

