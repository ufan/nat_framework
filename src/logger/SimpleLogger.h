#ifndef LOGGER_SIMPLE_LOG_H_
#define LOGGER_SIMPLE_LOG_H_

#include <stdio.h>

#define LOG_INFO(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_DBG(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_ERR(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_WARN(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)



#endif /* LOGGER_SIMPLE_LOG_H_ */
