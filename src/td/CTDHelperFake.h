/*
 * CTDHelperFake.h
 *
 *  Created on: 2018年5月14日
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDHELPERFAKE_H_
#define SRC_TD_CTDHELPERFAKE_H_

#include <queue>
#include <set>
#include <unordered_map>
#include "ITDHelper.h"

class CTDHelperFake: public ITDHelper
{
public:
	struct tTick
	{
		double last_px = 0.0;
		double ask = 0.0;
		double bid = 0.0;
		int	ask_vol = 0;
		int bid_vol = 0;
	};

	using ITDHelper::ITDHelper;
	virtual ~CTDHelperFake() {release();}

	bool init(const json& j_conf) {return true;}

	void doSendOrder(int track_id);

	void doDelOrder(int track_id);

	const tRtnMsg* doGetRtn();

	void release() {}

	void on_tick(const UnitedMarketData* pmd);

	virtual void on_switch_day(string day);

	bool makeMatch(int track_id, tTick &tick);

	tOrderTrack& getEngOrderTrack(int idx) {return eng_order_track_[idx];}

private:
	tRtnMsg							hold_result_;
	vector<tOrderTrack>				eng_order_track_;
	queue<tRtnMsg>					rtn_msg_queue_;
	unordered_map<uint32_t, tTick>	last_tick_wall_;
	set<int>						queued_track_id_;
};

#endif /* SRC_TD_CTDHELPERFAKE_H_ */
