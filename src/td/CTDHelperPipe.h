/*
 * CTDHelperPipe.h
 *
 *  Created on: Sep 7, 2018
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDHELPERPIPE_H_
#define SRC_TD_CTDHELPERPIPE_H_

#include <queue>
#include "ITDHelper.h"
#include "CPipExecutor.h"

class CTDHelperPipe: public ITDHelper
{
public:
	using ITDHelper::ITDHelper;
	virtual ~CTDHelperPipe() {release();}

	bool init(const json& j_conf);

	void doSendOrder(int track_id);

	void doDelOrder(int track_id);

	const tRtnMsg* doGetRtn();

	void release();

	virtual bool qryTradeBaseInfo();

	virtual bool qryOrderTrack();

	void writeJson(const json& j);

	json readJson();

protected:
	tRtnMsg				hold_result_;
	CPipExecutor 		*p_executor_ = nullptr;
	int					self_id_ 		= 0;
	queue<tRtnMsg>		rtn_msg_queue_;
};

#endif /* SRC_TD_CTDHELPERPIPE_H_ */
