/*
 * sotest.cpp
 *
 *  Created on: 2017年10月9日
 *      Author: hongxu
 */

#include <iostream>
#include <stdint.h>
#include <unistd.h>

extern "C" int run()
{
	sleep(30);
	return -2;
}

