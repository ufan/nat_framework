/*
 * CSystemIO.cpp
 *
 *  Created on: 2018年4月28日
 *      Author: sky
 */

#include "CSystemIO.h"

CSystemIO& CSystemIO::instance()
{
	static CSystemIO s_instance;
	return s_instance;
}

