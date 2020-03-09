/*
 * CGlobalParamter.h
 *
 *  Created on: 2017年12月20日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CGLOBALPARAMETER_H_
#define SRC_TRADER_CGLOBALPARAMETER_H_

#include <string>
#include <map>
#include "CConfig.h"
using namespace std;

class CGlobalParameter
{
public:
	CGlobalParameter();
	virtual ~CGlobalParameter();

	static CConfig* getConfig() { return &cnf_; }

	static bool initConfig(string file) { return cnf_.init(file); }

	static volatile bool terminal_lock_;

	static map<int, string> order_id_map_;

private:
	static CConfig cnf_;
};

#endif /* SRC_TRADER_CGLOBALPARAMETER_H_ */
