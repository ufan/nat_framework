/*
 * CTDHelperComm.h
 *
 *  Created on: 2018年5月8日
 *      Author: hongxu
 */

#ifndef SRC_TD_CTDHELPERCOMM_H_
#define SRC_TD_CTDHELPERCOMM_H_

#include "ITDHelper.h"
#include "CRawIOWriter.h"
#include "CRawIOReader.h"
#include "IOCommon.h"


class CTDHelperComm: public ITDHelper
{
public:
	using ITDHelper::ITDHelper;
	virtual ~CTDHelperComm() {release();}

	bool init(const json& j_conf);
	bool _init(string engine, int timeout=3);

	void doSendOrder(int track_id);

	void doDelOrder(int track_id);

	const tRtnMsg* doGetRtn();

	void release() {notifyTDEngineRemove();}

	virtual bool qryTradeBaseInfo();

	virtual bool qryOrderTrack();

	string createTdSendPath(string name);

	bool notifyTDEngineAdd();

	void notifyTDEngineRemove();

protected:
	int				self_id_ 		= 0;
	CRawIOWriter	td_writer_;
	CRawIOReader	td_reader_;
	int 			td_engine_id_ 	= 0;
	int				timeout_;
	string 			tdsend_path_;
};

#endif /* SRC_TD_CTDHELPERCOMM_H_ */
