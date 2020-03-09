/*
 * ioprotocol.h
 *
 *  Created on: 2018年4月23日
 *      Author: hongxu
 */

#ifndef SRC_IO_IOPROTOCOL_H_
#define SRC_IO_IOPROTOCOL_H_

#include <stdint.h>

#define PAGE_HEAD_VER_1		0x01
#define FRAME_HEAD_MAGIC	0x20180423
#define IO_PAGE_SIZE		(128 * 1024 * 1024)   // 128M


enum emPageStatus
{
	PAGE_STATUS_WRITTING,
	PAGE_STATUS_FINISH,
};


#pragma pack(1)

struct tPageHead
{
	uint8_t			status;
	uint32_t 		version;
	uint32_t		size;
	uint32_t		tail;
	long			start_nano;
	long			stop_nano;
	long			last_nano;
	uint64_t		reserved_;
	char			buf[0];
};

struct tFrameHead
{
	uint32_t		magic;
	uint32_t 		len;
	uint64_t		nano;
	uint32_t		reserved_;
	char			buf[0];
};

#pragma pack()


#endif /* SRC_IO_IOPROTOCOL_H_ */

