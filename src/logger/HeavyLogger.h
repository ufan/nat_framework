#ifndef LOGGER_HEAVY_LOGGER_H_
#define LOGGER_HEAVY_LOGGER_H_

#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#define LOG_INFO(format, ...) LOG4CPLUS_INFO_FMT(log4cplus::Logger::getRoot(), format, ##__VA_ARGS__)
#define LOG_DBG(format, ...) LOG4CPLUS_DEBUG_FMT(log4cplus::Logger::getRoot(), format, ##__VA_ARGS__)
#define LOG_ERR(format, ...) LOG4CPLUS_ERROR_FMT(log4cplus::Logger::getRoot(), format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG4CPLUS_WARN_FMT(log4cplus::Logger::getRoot(), format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) LOG4CPLUS_TRACE_FMT(log4cplus::Logger::getRoot(), format, ##__VA_ARGS__)

#define ASSERT(exp) do { if(!(exp)) LOG_ERR("ASSERT failed:"#exp); } while(0)
#define ASSERT_RET(exp, ret) do { if(!(exp)) {LOG_ERR("ASSERT failed:"#exp); return ret;} } while(0)


#define ENGLOG(format, ...)		LOG_INFO(format, ##__VA_ARGS__)
#define ALERT(format, ...)		LOG_ERR("ALERT|" format, ##__VA_ARGS__)


bool initLogger(std::string logconf);

#endif /* LOGGER_HEAVY_LOGGER_H_ */

