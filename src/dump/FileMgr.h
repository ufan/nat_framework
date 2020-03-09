#ifndef DUMP_FILEMGR_H
#define DUMP_FILEMGR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Structure.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class FileMgr
{
public:
	static bool init(json& j);
	static bool release();
	static bool regInstrument(const char* trading_date, vector<Instrument>& vec_instr);
	static FILE* getFilePtr(const char* instr_str);
	
	static FILE* pf_all;
	static unordered_map<string, string> map_instr_prd;
	static unordered_map<string, vector<Instrument>> map_prd_instr;
	static unordered_map<string, string> map_prd_exch;
	static unordered_map<string, FILE*> map_prd_file;
	static char warehouse[256];
};

#endif