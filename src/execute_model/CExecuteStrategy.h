/*
 * CExecuteStrategy.h
 *
 *  Created on: 2018年12月17日
 *      Author: hongxu
 */

#ifndef SRC_EXECUTE_MODEL_CEXECUTESTRATEGY_H_
#define SRC_EXECUTE_MODEL_CEXECUTESTRATEGY_H_

#include <unordered_map>
#include <memory>
#include "CStrategy.h"
#include "CLossModel.h"

class CExecuteStrategy: public CStrategy
{
public:
	CExecuteStrategy();
	virtual ~CExecuteStrategy();

	int execution(uint32_t instr_hash, double priceup, double pricedown, int placedsize, int direction, double duration);
	void cancelExec(uint32_t instr_hash, int execid);

protected:
    virtual void sys_on_tick(const UnitedMarketData* md);
    virtual void sys_on_time(long nano);
    virtual void sys_on_rtn(const tRtnMsg* rtn);
    virtual void sys_on_switch_day(string day);
    virtual void sys_on_subscribe(string instr);

protected:
	unordered_map<uint32_t, unique_ptr<CLossModel>>		loss_model_;
};

#endif /* SRC_EXECUTE_MODEL_CEXECUTESTRATEGY_H_ */
