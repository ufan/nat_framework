/*
 * IMDEngine.h
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#ifndef SRC_MD_IMDENGINE_H_
#define SRC_MD_IMDENGINE_H_

#include <string>
#include <vector>
#include <map>
#include "CRawIOWriter.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


class IMDEngine
{
public:
	enum emDayNightMode
	{
		MODE_DAY,
		MODE_NIGHT,
	};

	IMDEngine();
	virtual ~IMDEngine();

	string name() {return name_;}

	vector<string> querySubscribedInstrument();

	bool initEngine(const json& j_conf);

	void runEngine();

  void stop() { do_running_ = false; }

  void engine_subscribe(const vector<string> &instr);

  void engine_unsubscribe(const vector<string> &instr);

  void writeStartSignal();

  bool getBaseInfo(string td_engine_name, int timeout);

 protected:
	virtual bool init() = 0;

	virtual bool start() = 0;

	virtual void join() = 0;

	virtual void release() = 0;

	virtual void subscribe(const vector<string> &instr) = 0;

	virtual void unsubscribe(const vector<string> &instr) = 0;


protected:
	volatile bool		do_running_		= true;
	CSafeRawIOWriter 	md_writer_;
	int					self_id_ 		= 0;
	uint8_t				day_night_mode_ = MODE_DAY;
	string				name_;
	map<string, int>	subs_instr_;
	json				config_;
};

#endif /* SRC_MD_IMDENGINE_H_ */
