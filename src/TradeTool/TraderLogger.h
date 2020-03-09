/*
 * TraderLogger.h
 *
 *  Created on: 2017年12月20日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_TRADERLOGGER_H_
#define SRC_TRADER_TRADERLOGGER_H_

#include <stdio.h>

#define LOG_INFO(format, ...) /* fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__) */
#define LOG_DBG(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_ERR(format, ...) fprintf(stderr, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_WARN(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#define LOG_TRACE(format, ...) fprintf(stdout, "%s:%d|" format "\n", __FILE__, __LINE__, ## __VA_ARGS__)


#endif /* SRC_TRADER_TRADERLOGGER_H_ */
