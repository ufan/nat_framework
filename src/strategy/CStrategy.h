/*
 * CStrategy.h
 *
 *  Created on: May 25, 2018
 *      Author: hongxu
 */

#ifndef SRC_STRATEGY_CSTRATEGY_H_
#define SRC_STRATEGY_CSTRATEGY_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ATStructure.h"
#include "IMDHelper.h"
#include "ITDHelper.h"
#include "BarMaker.h"
#include "BarHelper.h"
#include "RiskStg.h"
#include "CSystemIO.h"
using namespace std;

template<class K, class V>
using umap = unordered_map<K, V>;


class CStrategy
{
public:
	CStrategy();
	virtual ~CStrategy();

	virtual void on_tick(const UnitedMarketData*) {}
	virtual void on_time(long nano) {}
	virtual void on_rtn(const tRtnMsg*) {}
	virtual void on_bar(const Bar*) {}
	virtual void on_switch_day(string day) {}
	virtual void on_msg(const char *p, uint32_t len) {}

	bool init(string config_file);
	bool initStr(string config_content);
	void run();
	void stop();
	void release();

	bool subscribe(string instruments);
	bool subsBar(string instrument, long interval_min);
	void setIntraDayBaseSec(long sec) {BarHelper::setIntraDayBaseSec(sec);}

	int sendOrder(uint32_t instr_hash, double price, int vol, int dir, int off, int acc_idx=0);
	int sendOrder(string instr, double price, int vol, int dir, int off, int acc_idx=0)
	{
		return sendOrder(INSTR_STR_TO_HASH(instr), price, vol, dir, off, acc_idx);
	}

	int cancelOrder(int order_id);

	void setReadPos(long nano) {p_md_helper_->setReadPos(nano);}
	const UnitedMarketData* readMd();

	const tLastTick* getLastTick(uint32_t instr_hash)
	{
		auto itr = instr_info_map_.find(instr_hash);
		return itr != instr_info_map_.end() ? &(itr->second.lst_tick) : nullptr;
	}

	const tLastTick* getLastTick(string instr)
	{
		return getLastTick(INSTR_STR_TO_HASH(instr));
	}

	int getOrderTrackCnt() {return p_td_helper_->getOrderTrackCnt();}

	tOrderTrack& getOrderTrack(int idx) {return p_td_helper_->getOrderTrack(idx);}

	string getTradingDay() {return trading_day_;}

	const RiskStg* getAccountObj() {return p_risk_stg_.get();}

	void barOnFinish();

private:
	void processTick(const UnitedMarketData* pmd);
	void processSysIO();

	bool loadConfig(string config_content);
	void pre_on_switch_day(string day);

protected:
	virtual void sys_on_start() {}
	virtual void sys_on_tick(const UnitedMarketData*) {}
	virtual void sys_on_time(long nano) {}
	virtual void sys_on_rtn(const tRtnMsg*) {}
	virtual void sys_on_switch_day(string day) {}
	virtual void sys_on_stop() {}
	virtual void sys_on_subscribe(string instr) {}

private:
	unique_ptr<IMDHelper>					p_md_helper_;
	unique_ptr<ITDHelper>					p_td_helper_;
	unique_ptr<RiskStg> 					p_risk_stg_;
	umap<uint32_t, tSubsInstrInfo>			instr_info_map_;
	vector<BarMaker>						subs_bar_;

	volatile uint32_t*						p_is_exit_ 	= nullptr;
	volatile uint32_t*						p_do_trade_ = nullptr;
	string									trading_day_;
	string									acc_save_day_;

	int										self_id_ = 0;
	unique_ptr<CRawIOReader>				p_sys_io_reader_;
};

#endif /* SRC_STRATEGY_CSTRATEGY_H_ */

