#include "CLossModel.h"
#include "CStrategy.h"
#include <stdlib.h>
#include "json.hpp"
#include <fstream>

int log_id = 0;
std::string tradingId = "";

int main(int argc, char *argv[])
{
	CLossModel lm;
    std::ifstream f1(argv[1]);
    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();
    json reader = json::parse(content);
    tradingId = argv[2];
    
	if(!lm.initStr(reader.dump()))
	{
		cerr << "strategy init err" << endl;
		return -1;
	}
	
    log_id = getNewLogger(tradingId);    
    lm.subscribe(tradingId);
    
	lm.run();
	lm.release();
	
	return 0;
}