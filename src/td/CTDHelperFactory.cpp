/*
 * CTDHelperFactory.cpp
 *
 *  Created on: 2018年5月10日
 *      Author: hongxu
 */

#include "CTDHelperFactory.h"
#include "CTDHelperComm.h"
#include "CTDHelperFake.h"
#include "CTDHelperPipe.h"
#include "CTDHelperPython.h"

ITDHelper* CTDHelperFactory::create(string type, string name)
{
	if(type == "comm")
	{
		return new CTDHelperComm(name);
	}
	else if(type == "fake")
	{
		return new CTDHelperFake(name);
	}
	else if(type == "pipe")
	{
		return new CTDHelperPipe(name);
	}
    else if(type == "python")
    {
        return new CTDHelperPython(name);
    }
	return nullptr;
}
