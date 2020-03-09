#ifndef SRC_TESTSUITE_OTMONITOR_H
#define SRC_TESTSUITE_OTMONITOR_H

#include <map>
#include "ATStructure.h"

using namespace std;

class OTMonitorHelper
{
public:
	bool readOT(const char* path, int bgn, int end);
	
	map<long, tOrderTrack> map_ot;
};

#endif
