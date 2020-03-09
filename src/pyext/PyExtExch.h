/*
 * PyExtExch.h
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 */

#ifndef SRC_PYEXT_PYEXTEXCH_H_
#define SRC_PYEXT_PYEXTEXCH_H_

#include <string>
#include <vector>
#include <boost/python.hpp>
#include "ATStructure.h"
#include "json.hpp"
using namespace std;
namespace bp = boost::python;
using json = nlohmann::json;


class CPyExtTdExch
{
public:
	CPyExtTdExch();
	virtual ~CPyExtTdExch();

public:
	bool init(const json &j_conf);
	void sendOrder(tOrderTrack ot);
	void delOrder(int track_id);
	bool qryTradeBaseInfo();
	bool qryOrderTrack(vector<tOrderTrack> &ot);
	const tRtnMsg* getRtn();
	void on_tick(const UnitedMarketData *pumd);
	void on_time(long nano);
	void on_switch_day(string day);
	void release();

protected:
	bp::object		td_send_order_;
	bp::object		td_del_order_;
	bp::object		td_qry_base_info_;
	bp::object		td_qry_order_track_;
	bp::object		td_get_rtn_;
	bp::object		td_on_tick_;
	bp::object		td_on_time_;
	bp::object		td_switch_day_;
	bp::object		td_release_;
	tRtnMsg			hold_rtn_;
};

class CPyExtMdExch
{
public:
	CPyExtMdExch();
	virtual ~CPyExtMdExch();

public:
	bool init(const json &j_conf);
	const UnitedMarketData* readMd(long &md_nano);
	vector<string> getSubsInstr();
    bool subscribe(const vector<string> &instr);
    bool unsubscribe(const vector<string> &instr);
    void setReadPos(long nano);
	void release();

protected:
	bp::object			md_read_;
	bp::object			md_get_subs_;
	bp::object			md_subs_;
	bp::object			md_unsubs_;
	bp::object			md_set_read_pos_;
	bp::object			md_release_;
	UnitedMarketData	hold_tick_;
};

#endif /* SRC_PYEXT_PYEXTEXCH_H_ */
