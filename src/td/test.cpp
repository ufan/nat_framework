/*
 * test.cpp
 *
 *  Created on: 2018年5月9日
 *      Author: hongxu
 */

//#include <iostream>
//#include "CTDHelperComm.h"
//#include "Logger.h"
#include <string>
#include <dlfcn.h>

using namespace std;

long getCurMdTime()
{
	return 0;
}

//void test()
//{
//	CTDHelperComm td_helper;
//
//	json j_conf = R"({
//	"helper_name": "TDHelper_test",
//	"td_engine_name" : "CTPTD_ENGINE_V1"
//	})"_json;
//
//	if(!td_helper.init(j_conf)) return;
//
//	cout << "connect to td succ" << endl;
//
//}

void* findSymbol(const char* path, const char* symbol)
{
	void* handle = dlopen(path, RTLD_LAZY);
	if (!handle)
	{
		printf("can't open %s, error: %s\n", path, dlerror());
		return NULL;
	}
	
	void* target = dlsym(handle, symbol);
	if (!target)
	{
		printf("can't find symbol %s, error: %s\n", symbol, dlerror());
		return NULL;
	}
	
	return target;
}

ITDEngine* createEngine(string so_path, string type)
{
	if(type == "ctp")
	{
		tfpCreateTDEngineCTP fpCreateTDEngineCTP = (tfpCreateTDEngineCTP)findSymbol((so_path + "/libtdctp.so").c_str(), "createTDEngineCTP");
		if (fpCreateTDEngineCTP)
			return fpCreateTDEngineCTP();
	}
	else if(type == "xt")
	{
		tfpCreateTDEngineXt fpCreateTDEngineXt = (tfpCreateTDEngineXt)findSymbol((so_path + "/libtdxt.so").c_str(), "createTDEngineXt");
		if (fpCreateTDEngineXt)
			return fpCreateTDEngineXt();
	}
	else
	{
		printf("unknown engine type: %s\n", type.c_str());
	}
	
	return nullptr;
}

void test1()
{
	ITDEngine* eng = createEngine("/home/jarvis/TDEngine/bin", "xt");
	if (eng != NULL)
	{
		eng->connect(5, "172.22.4.103:65300");
		printf("%s\n", "succ");
	}
}

int main()
{
//	initLogger("./logger.cnf");
	test1();
	getchar();
	return 0;
}

