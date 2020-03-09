/*
 * CWareHouseReader.h
 *
 *  Created on: 2018年8月2日
 *      Author: hongxu
 */

#ifndef SRC_COMMON_CWAREHOUSEREADER_H_
#define SRC_COMMON_CWAREHOUSEREADER_H_

#include <string>
#include <vector>
#include <memory>
#include <limits.h>
#include "IOCommon.h"
#include "Instrument.h"
using namespace std;

class CWareHouseFileCursor
{
public:
	#pragma pack(push, 1)
	struct tFileHeadV1
	{
		int 		file_ver;
		int 		md_head_ver;
		int 		md_body_ver;
		char 		system_date[16];
		char		trading_date[16];
		char 		system_day_night[32];
		int 		inst_cnt;
		Instrument	inst_arr[0];
	};
	#pragma pack(pop)

public:
	CWareHouseFileCursor(string file);
	virtual ~CWareHouseFileCursor();
	void close();
	void getInstrumentInfo(vector<Instrument> &res);
	int next();

public:
	int 				fd_ = -1;
	tFileHeadV1 		file_head_;
	long				local_time_ = LONG_MAX;
	tIOMarketData 		umd_;
	string				file_;
};

class CWareHouseReader
{
public:
	CWareHouseReader();
	virtual ~CWareHouseReader();

	void openFile(string file);

	void switchDay();

	int merge(tIOMarketData &data_rtn);

	void loadFileList(string file_pattern, string start_day, string end_day);

	void loadFiles(vector<string> file_patterns, string start_day, string end_day);

	virtual void onSwitchDay(string instrInfo) {}

	const UnitedMarketData* readTick();

	void setReadPos(long nano);

	void clear()
	{
		files_.clear();
		cur_trading_day_.clear();
	}

	void doSetTimer(bool flag) {do_set_time_ = flag;}

protected:
	tIOMarketData 		tick_;
	vector<unique_ptr<CWareHouseFileCursor>> files_;
	long				local_time_ = 0;
	string 				cur_trading_day_;
	bool				do_set_time_ = false;
	vector<string>		file_patterns_;
	string 				start_day_;
	string 				end_day_;
};

#endif /* SRC_COMMON_CWAREHOUSEREADER_H_ */
