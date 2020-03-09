#ifndef SRC_ACCOUNT_CONFUTIL_H_
#define SRC_ACCOUNT_CONFUTIL_H_

#include "json.hpp"
#include <string>
#include "Logger.h"

using namespace std;
using json = nlohmann::json;

template<class T>
class ConfUtil {
public:
	static bool getValue(T* ret, const json& j_conf, string prd_str, string instr_str) {
		if (j_conf.find(instr_str) != j_conf.end()) {
			*ret = j_conf[instr_str];
		} else if (j_conf.find(prd_str) != j_conf.end()) {
			*ret = j_conf[prd_str];
		} else if (j_conf.find("default") != j_conf.end()) {
			*ret = j_conf["default"];
		} else {
			ALERT("Can't find config node");
			return false;
		}
		return true;
	}
};

#endif