#ifndef SRC_TESTSUITE_IOMONITOR_H
#define SRC_TESTSUITE_IOMONITOR_H

#include "stdint.h"
#include "ATStructure.h"
#include <vector>
#include <string>
using namespace std;

class IOMonitorHelper
{
public:
	const char* getCmdString(int cmd);
	void printBuf(const char *buf, uint32_t len);
	void printSysIOHead(const char* p, uint32_t len);
	void printIOMarketData(const char* p, uint32_t len);
	void printIOInputOrder(const char* p, uint32_t len);
	void printIOrderAction(const char* p, uint32_t len);
	void printIOrderRtn(const char* p, uint32_t len);
	void printIOTDBaseInfo(const char* p, uint32_t len);
	void printIOMArketTradingDay(const char* p, uint32_t len);
	void printIOrderTrack(const char* p, uint32_t len);
	void printCmdContent(const char* p, uint32_t len);
	string printOrderRtnContent(tRtnMsg* p_rtn_msg);
	void readIO(const char *file, int from_no, long from_nano, long to_nano);
	
	vector<tRtnMsg> vec_rtn;
};

#endif
