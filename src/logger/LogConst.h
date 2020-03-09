/*
 * LogConst.h
 *
 *  Created on: 2017年11月6日
 *      Author: hongxu
 */

#ifndef LIB_LOG_LOGCONST_H_
#define LIB_LOG_LOGCONST_H_

#include "CLogPool.h"
#include "Log.h"
#include "EngineComm.h"
#include <string>
#include "UtilClass.h"
#include "Config.h"

#define RELEASE

#ifdef RELEASE
#define DEBUGLOG(format, ...)
#else
#define DEBUGLOG(format, ...) LOG_DBG(format, ##__VA_ARGS__)
#endif

#define MDLOG(format, ...) do{DEBUGLOG(format, ##__VA_ARGS__); LOG(0, "%s| " format, UtilClass::GetNanoString(UtilClass::GetNano()).c_str(), ##__VA_ARGS__);}while(0)
#define TDLOG(format, ...) do{DEBUGLOG(format, ##__VA_ARGS__); LOG(1, "%s| " format, UtilClass::GetNanoString(UtilClass::GetNano()).c_str(), ##__VA_ARGS__);}while(0)
#define ENGLOG(format, ...) do{DEBUGLOG(format, ##__VA_ARGS__); LOG(2, "%s| " format, UtilClass::GetNanoString(UtilClass::GetNano()).c_str(), ##__VA_ARGS__);}while(0)
#define ALERT(format, ...) do{DEBUGLOG(format, ##__VA_ARGS__); LOG(3, "ALERT|%s| " format, UtilClass::GetNanoString(UtilClass::GetNano()).c_str(), ##__VA_ARGS__);}while(0)

#define MDLOG_RAW(format, ...) do{LOG(0, format, ##__VA_ARGS__);}while(0)
#define TDLOG_RAW(format, ...) do{LOG(1, format, ##__VA_ARGS__);}while(0)
#define ENGLOG_RAW(format, ...) do{LOG(2, format, ##__VA_ARGS__);}while(0)
#define ALERT_RAW(format, ...) do{LOG(3, format, ##__VA_ARGS__);}while(0)

using namespace std;

inline void initLogPool(const char *cfg_dir)
{
	Config::InitLogCfg(cfg_dir);
	int mdid = CLogPool::instance()->getNewLoggerID(string(Config::log_path) + string(Config::log_prefix) + "md_", LOG_DAY);
	int tdid = CLogPool::instance()->getNewLoggerID(string(Config::log_path) + string(Config::log_prefix) + "td_", LOG_DAY);
	int engid = CLogPool::instance()->getNewLoggerID(string(Config::log_path) + string(Config::log_prefix) + "eng_", LOG_DAY);
	int alertid = CLogPool::instance()->getNewLoggerID(string(Config::log_path) + string(Config::log_prefix) + "alert_", LOG_DAY);
	CLogPool::instance()->startLogThread();
}

#endif /* LIB_LOG_LOGCONST_H_ */
