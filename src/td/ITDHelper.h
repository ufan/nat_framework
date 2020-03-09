/*
 * ITDHelper.h
 *
 *  Created on: 2018年4月26日
 *      Author: hongxu
 */

#ifndef SRC_TD_ITDHELPER_H_
#define SRC_TD_ITDHELPER_H_

#include <memory>
#include <string>
#include <vector>
#include "MurmurHash2.h"
#include "ATStructure.h"
#include "Logger.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class ITDHelper
{
public:
	ITDHelper(string name);
	virtual ~ITDHelper() {}

	virtual bool init(const json& j_conf) = 0;

	int sendOrder(const char *instr, double price, int vol, int dir, int off, int acc_id, int stgid, uint32_t instr_hash);

	int deleteOrder(int id);

	const tRtnMsg* getRtn();

	int getOrderTrackCnt() {return order_track_.size();}

	tOrderTrack& getOrderTrack(int idx) {return order_track_[idx];}

	int	getStgIdFromRtnMsg(const tRtnMsg* prtn) {return getOrderTrack(prtn->local_id).stg_id;}

	void closeOrderTrack();

	virtual void doSendOrder(int track_id) = 0;
	virtual void doDelOrder(int track_id) = 0;
	virtual const tRtnMsg* doGetRtn() = 0;
	virtual void release() {}
	virtual bool qryTradeBaseInfo() {return false;}		// 获取基础信息，填写 trading_day 及 instruments info
	virtual bool qryOrderTrack() {return false;}

	virtual void on_tick(const UnitedMarketData* pmd) {}
	virtual void on_time(long nano) {}
	virtual void on_switch_day(string day) {closeOrderTrack();}

public:
	int sendOrder(string instr, double price, int vol, int dir, int off, int acc_id, int stgid=0)
	{
		return sendOrder(instr.c_str(), price, vol, dir, off, acc_id, stgid, INSTR_STR_TO_HASH(instr));
	}

	bool initStr(string conf) {return init(json::parse(conf));}

protected:
	vector<tOrderTrack>	order_track_;
	string name_;
};

typedef unique_ptr<ITDHelper>  ITDHelperPtr;

#endif /* SRC_TD_ITDHELPER_H_ */
