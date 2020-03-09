/*
 * ITDEngine.h
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 *
 *  Note: TDEngine part COPY from kungfu
 *
 */

#ifndef SRC_TD_ITDENGINE_H_
#define SRC_TD_ITDENGINE_H_

#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "MurmurHash2.h"
#include "CReaderPool.h"
#include "SysConf.h"
#include "IOCommon.h"
#include "CRawIOWriter.h"
#include "Logger.h"
#include "json.hpp"
#include "CTradeBaseInfo.h"
#include "COrderTrackMmap.h"
#include "RiskTop.h"

using namespace std;
using json = nlohmann::json;

class ITDEngine
{
public:
	template<int N=4>  // N must be power of 2
	class CIDQueue
	{
	public:
		bool test(long long m)
		{
			int s = front < N ? front : N;
			for(int i = 0; i < s; i++)
			{
				if(m == id[i]) return true;
			}
			id[front++ & (N-1)] = m;
			return false;
		}
	public:
		int front = 0;
		long long id[N];
	};

public:
	ITDEngine();
	virtual ~ITDEngine();

	virtual string name() = 0;

	virtual bool init(const json& j_conf) = 0;

    virtual void release() = 0;

    virtual int getAccountCnt() = 0;

    virtual bool updateOrderTrack() = 0;

public:
	bool initEngine(const json& j_conf);

	bool initAccountUtilis(const json& j_conf);

	bool loadOrderTrack();

	void writeStartSignal();

    void listening();

    void stop() { do_running_ = false; release(); }

    bool checkRequestId(int id) {return request_id_start_ <= id && id <= request_id_end_;}

    tOrderTrack& get_request_track(int id)
	{
		return request_track_[id & (MMAP_ORDER_TRACK_SIZE - 1)];
	}

    bool testOtId(int otid, long long mid)
    {
    	while(flag_.test_and_set(memory_order_acquire));
    	auto res = otidfilter_[otid & (MMAP_ORDER_TRACK_SIZE - 1)].test(mid);
    	flag_.clear(memory_order_release);
    	return res;
    }

	void writeErrRtn(const tIOInputOrderField* data, int errid, const char* msg, int msgtp=ODS(ERR));

	void writeErrRtn(const tIOrderAction* data, int errid, const char* msg, int msgtp=ODS(ERR));

	tIOrderRtn* writeRtnFromTrack(tOrderTrack &request_track);

	void engine_req_order_action(const tIOrderAction* data);

	// this should be called after return msg
	void engine_on_rtn(int acc_idx, const tOrderTrack *pot, const tRtnMsg *prtn)
	{
		acc_utilis_[acc_idx]->onRtn(pot, prtn);
	}

	void engine_on_close();

	void rspOrderTrack(tSysIOHead *req);

public:
    /** is every accounts connected? */
    virtual bool is_connected() const = 0;
    /** is every accounts logged in? */
    virtual bool is_logged_in() const = 0;

public:
    /** insert order */
    virtual void req_order_insert(const tIOInputOrderField* data) = 0;

    /** request order action (only cancel is accomplished) */
    virtual void req_order_action(const tIOrderAction* data) = 0;

    virtual bool getBaseInfo() = 0;		// 获取基础信息，更新 CTradeBaseInfo

protected:
    volatile bool						do_running_  = 	true;
	int									self_id_	 = 	0;
	CReaderPool							read_pool_;
	CSafeRawIOWriter					writer_;
	vector<unique_ptr<RiskTop> >		acc_utilis_;
    int 								request_id_ 	= 1;
    int									request_id_start_ = 1;
    int 								request_id_end_ = 1000000;
	tOrderTrack							*request_track_ = nullptr;
	COrderTrackMmap						otmmap_{true};
	CIDQueue<>							otidfilter_[MMAP_ORDER_TRACK_SIZE];

	static atomic_flag					flag_;
};

#endif /* SRC_TD_ITDENGINE_H_ */

