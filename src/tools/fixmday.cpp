/*
 * fixmday.cpp
 *
 *  Created on: 2018年11月2日
 *      Author: hongxu
 */

#include <stdio.h>
#include <string>
#include <string.h>
#include "CRawIOModify.h"
#include "ioprotocol.h"
#include "Logger.h"
#include "IOCommon.h"
#include "CTimer.h"
using namespace std;


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s io_file\n", argv[0]);
		exit(-1);
	}
	initLogger("./logger.cnf");

	CRawIOModify io;
	ASSERT_RET(io.loadFile(argv[1]), -1);

	uint32_t this_size;
	tPageHead *this_head = (tPageHead*)(io.getRawBuffer(this_size));
	long this_lst_nano = this_head->last_nano;

	const tFrameHead *lst = nullptr;
	const tFrameHead *nearest = nullptr;
	const tFrameHead *lst_info = nullptr;
	while(const tFrameHead* p = io.getCurFrame())
	{
		if(*(int*)(p->buf) == IO_TD_RSP_BASE_INFO)
		{
			printf("find trading day: %s\n", ((tIOTDBaseInfo*)((tSysIOHead*)p->buf)->data)->trading_day);
			lst_info = p;
			nearest = lst;
		}
		if(p->nano >= this_lst_nano) break;
		lst = p;
		io.passFrame();
	}

	if(lst_info)
	{
		if(nearest)
		{
			const char *p_day = ((tIOTDBaseInfo*)((tSysIOHead*)(lst_info->buf))->data)->trading_day;
			printf("find nearest trading day at %s. data after(include) this day will be discarded. confirm[y|n]:", p_day);
			char c; scanf("%c", &c);
			if(c == 'y')
			{
				this_head->last_nano = nearest->nano;
				this_head->tail = (long)lst_info - (long)this_head;
				this_head->stop_nano = -1;
				this_head->status = PAGE_STATUS_WRITTING;
				memset((void*)lst_info, 0, this_head->size - this_head->tail);
			}
			printf("FINISHED.\n");
		}
		else
		{
			printf("you can remove this file and those bigger files directly.\n");
		}
	}
	else
	{
		LOG_ERR("not found trading day begin point...");
	}

	io.unload();
	return 0;
}


