/*
 * Log.h
 *
 *  Created on: Sep 6, 2017
 *      Author: hongxu
 */
#include <stdio.h>

#ifndef LIB_COMMON_LOG_H_
#define LIB_COMMON_LOG_H_


#define LOG_INFO(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_DBG(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_ERR(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_WARN(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)



#endif /* LIB_COMMON_LOG_H_ */
