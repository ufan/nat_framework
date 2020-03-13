#ifndef LOGGER_FASTLOGGER_H_
#define LOGGER_FASTLOGGER_H_

#include "CLogPool.h"
#include "json.hpp"
using json = nlohmann::json;

extern int g_englogger_id_;

void initFastLogger(const json &j_conf);

int getNewLogger(string path);

void fastLog(int id, string log);

int getStgLoggerId();

void initFastLoggerConf(int buf_size_kb, string file_mode, string write_mode, int thread_cycle_us);

#define FASTLOG_DBG         10000
#define FASTLOG_INFO        10001
#define FASTLOG_WARN        10002
#define FASTLOG_ERR         10003
#define FASTLOG_TRACE       10004

extern int g_fastlogger_level_;

#define LOG_LVL(level, format, ...) do{if(g_fastlogger_level_ <= level) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)

#define LOG_INFO(format, ...) do{if(g_fastlogger_level_ <= FASTLOG_INFO) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)
#define LOG_DBG(format, ...) do{if(g_fastlogger_level_ <= FASTLOG_DBG) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)
#define LOG_ERR(format, ...) do{if(g_fastlogger_level_ <= FASTLOG_ERR) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)
#define LOG_WARN(format, ...) do{if(g_fastlogger_level_ <= FASTLOG_WARN) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)
#define LOG_TRACE(format, ...) do{if(g_fastlogger_level_ <= FASTLOG_TRACE) LOG(g_englogger_id_, format, ##__VA_ARGS__);}while(0)

#define ASSERT(exp) do { if(!(exp)) LOG_ERR("ASSERT failed:"#exp); } while(0)
#define ASSERT_RET(exp, ret) do { if(!(exp)) {LOG_ERR("ASSERT failed:"#exp); return ret;} } while(0)


#define ENGLOG(format, ...)		LOG_INFO(format, ##__VA_ARGS__)
#define ALERT(format, ...)		LOG_ERR("ALERT|" format, ##__VA_ARGS__)


#endif /* LOGGER_FASTLOGGER_H_ */
