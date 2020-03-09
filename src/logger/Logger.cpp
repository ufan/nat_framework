/*
 * Logger.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: hongxu
 */

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include "Logger.h"
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/consoleappender.h>
using namespace log4cplus;
using namespace std;


bool initLogger(string logconf)
{
	if(access(logconf.c_str(), F_OK) == 0)
	{
		PropertyConfigurator::doConfigure(logconf);
	}
	else
	{
		cerr << "can't open log4cplus config file: "
				<< logconf << ". use console for output." << endl;

		// config default logger
		SharedAppenderPtr appender(new ConsoleAppender());

		unique_ptr<Layout> layout(new PatternLayout("%D|%l|%p|%m%n"));

		appender->setLayout(move(layout));

		Logger logger = Logger::getRoot();

		logger.setLogLevel(ALL_LOG_LEVEL);

		logger.addAppender(appender);
	}
	return true;
}

