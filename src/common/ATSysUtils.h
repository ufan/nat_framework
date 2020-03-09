/*
 * ATSysUtils.h
 *
 *  Created on: May 29, 2018
 *      Author: hongxu
 */

#ifndef SRC_COMMON_ATSYSUTILS_H_
#define SRC_COMMON_ATSYSUTILS_H_

#include <string>
#include "IOCommon.h"
#include "json.hpp"
using namespace std;
using json = nlohmann::json;


string sysRequest(int cmd, int rsp_cmd, int to, int source, int timeout, const void *extra_data=nullptr, uint32_t extra_len=0);

json toJson(const UnitedMarketData *p);
void parseJsonStr(const json &j, UnitedMarketData *save);

json toJson(const tInstrumentInfo *p);
void parseJsonStr(const json &j, tInstrumentInfo *save);

json baseInfo2Json();
void json2BaseInfo(const json &j);

void parseJsonStr(const json &j, tRtnMsg *save);

#endif /* SRC_COMMON_ATSYSUTILS_H_ */
