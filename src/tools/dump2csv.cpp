/*
 * dump2csv.cpp
 *
 *  Created on: 2018年3月12日
 *      Author: hongxu
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <iostream>
#include <stdio.h>
#include "Instrument.h"
#include "OldUnitedMarketData.h"
#include "utils.h"
using namespace std;

#pragma pack(push, 1)
struct tFileHeadV1
{
	int 		file_ver;
	int 		md_head_ver;
	int 		md_body_ver;
	char 		system_date[32];
	char 		system_day_night[32];
	int 		inst_cnt;
	Instrument	inst_arr[0];
};
#pragma pack(pop)


FILE *g_csv_file = NULL;

bool parseFileHeadV1(int fd, tFileHeadV1 *head)
{
	int len = head->inst_cnt * sizeof(Instrument);
	if(lseek(fd, len, SEEK_CUR) < 0)
	{
		perror("file format err:");
		return false;
	}
	return true;
}

long mkExchTime(long local_time, const char *update_time, int update_millisec)
{
	long exch_time = (local_time + 8 * 3600000000000L) / 86400000000000L * 86400000000000L +
			getSecondsFromClockStr(update_time) * 1000000000L + update_millisec * 1000000L - 8 * 3600000000000L;
	if(exch_time > local_time + 2 * 3600000000000L) exch_time -= 86400000000000L;
	else if(exch_time < local_time - 20 * 3600000000000L) exch_time += 86400000000000L;
	return exch_time;
}

int transMD(int fd, int body_ver, long local_time)
{
	int iread = 0;
	switch (body_ver)
	{
		case UnitedMarketDataType::CTP_v638:
		{
			CThostFtdcDepthMarketDataField_v638 data;
			iread = read(fd, &data, sizeof(data));
			if(iread == 0) return 0;
			else if(sizeof(data) != iread) return -1;
			fprintf(g_csv_file, "%ld,%s,%s,%d,%lf,%d,%s,%lf,%lf,%lf,%d,%lf,%d\n", mkExchTime(local_time, data.UpdateTime, data.UpdateMillisec), data.InstrumentID,
					data.UpdateTime, data.UpdateMillisec, data.LastPrice, data.Volume, data.ExchangeID, data.Turnover,
					data.OpenInterest, data.AskPrice1, data.AskVolume1, data.BidPrice1, data.BidVolume1);
			break;
		}
		case UnitedMarketDataType::EES_v20135:
		{
			EESMarketDepthQuoteData_v20135 data;
			iread = read(fd, &data, sizeof(data));
			if(iread == 0) return 0;
			else if(sizeof(data) != iread) return -1;
			fprintf(g_csv_file, "%ld,%s,%s,%d,%lf,%d,%s,%lf,%lf,%lf,%d,%lf,%d\n", mkExchTime(local_time, data.UpdateTime, data.UpdateMillisec), data.InstrumentID,
					data.UpdateTime, data.UpdateMillisec, data.LastPrice, data.Volume, data.ExchangeID, data.Turnover,
					data.OpenInterest, data.AskPrice1, data.AskVolume1, data.BidPrice1, data.BidVolume1);
			break;
		}
		case UnitedMarketDataType::EES_sf_v428:
		{
			guava_udp_normal_v428 data;
			iread = read(fd, &data, sizeof(data));
			if(iread == 0) return 0;
			else if(sizeof(data) != iread) return -1;
			fprintf(g_csv_file, "%ld,%s,%s,%d,%lf,%d,%c,%lf,%lf,%lf,%d,%lf,%d\n", mkExchTime(local_time, data.m_update_time, data.m_millisecond), data.m_symbol,
					data.m_update_time, data.m_millisecond, data.m_last_px, data.m_last_share, data.m_exchange_id, data.m_total_value,
					data.m_total_pos.m_shfe, data.m_ask_px, data.m_ask_share, data.m_bid_px, data.m_bid_share);
			break;
		}
		default:
			cerr << "unknown md_body version " << body_ver << endl;
			return false;
	}
	return iread;
}

bool transFileV1(int fd)
{
	tFileHeadV1 file_head;
	if(sizeof(file_head) != read(fd, &file_head, sizeof(file_head)))
	{
		cerr << "file format err" << endl;
		return false;
	}
	if(!parseFileHeadV1(fd, &file_head)) return false;

	while(1)
	{
		long local_time = 0;
		switch(file_head.md_head_ver)
		{
		case 1:
		{
			MarketDataHead_v1 md_head;
			int iread = read(fd, &md_head, sizeof(md_head));
			if(iread == 0) break;
			else if(sizeof(md_head) != iread)
			{
				cerr << "file incomplete." << endl;
				return false;
			}
			local_time = md_head.local_time;
			fprintf(g_csv_file, "%ld,", md_head.local_time);
			break;
		}
		default:
			cerr << "unknown md_head version " << file_head.md_head_ver << endl;
			return false;
		}

		int iret = transMD(fd, file_head.md_body_ver, local_time);
		if(iret < 0)
		{
			cerr << "md data incomplete " << iret << endl;
			return false;
		}
		else if(iret == 0) break;
	}

	return true;
}

void transFile(char *path)
{
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	if(fd < 0)
	{
		perror("open file err:");
		return;
	}

	int file_ver;
	if(sizeof(int) != read(fd, &file_ver, sizeof(int)))
	{
		cerr << "file format err" << endl;
		return;
	}
	lseek(fd, 0, SEEK_SET);

	switch(file_ver)
	{
	case 1:
		 transFileV1(fd);
		 break;
	default:
		cerr << "unknown file version " << file_ver << endl;
		return;
	}

	close(fd);
}

bool createCsvFile(char *path)
{
	g_csv_file = fopen(path, "w");
	if(g_csv_file == NULL)
	{
		perror("create csv file err:");
		return false;
	}

	fprintf(g_csv_file, "LocalTime,ExchTime,InstrumentID,UpdateTime,UpdateMillisec,LastPrice,Volume,ExchangeID,Turnover,OpenInterest,AskPrice1,AskVolume1,BidPrice1,BidVolume1\n");
	return true;
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Usage: %s dump_file dst_csv_file\n", argv[0]);
		return -1;
	}

	if(!createCsvFile(argv[2])) return -1;
	transFile(argv[1]);

	return 0;
}

