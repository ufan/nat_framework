#ifndef SRC_ACCOUNT_CONFUTIL_H_
#define SRC_ACCOUNT_CONFUTIL_H_

#include <string>

#include "Logger.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

template <class T>
class ConfUtil {
 public:
  /**
   * @brief Get value from json based on hierarchy keys
   * @details Key priority: instrument > product > default
   * @param[out] ret Save the value in this variable
   * @param[in] j_conf JSON configure
   * @param[in] prd_str  Product name
   * @param[in] instr_str  Instrument name
   * @return False if no field exists in the configure file
   */
  static bool getValue(T* ret, const json& j_conf, string prd_str,
                       string instr_str) {
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
