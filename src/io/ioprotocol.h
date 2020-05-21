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
	PAGE_STATUS_WRITTING, // space still available
	PAGE_STATUS_FINISH, // space limited, the Page is closed
};


#pragma pack(1)

// Each Page starts with a tPageHead and then followed by data frames
struct tPageHead
{
	uint8_t			status; // status of this Page, writing or finished
	uint32_t 		version; // version number of the Page format
	uint32_t		size; // size of this Page
	uint32_t		tail; // length of occupied buffer in this Page
	long			start_nano; // When this Page is created
	long			stop_nano; // When this Page is finished, i.e. full and closed
	long			last_nano; // The latest time new Frame is filled into this Page
	uint64_t		reserved_; // ?, unused
	char			buf[0]; // the data frames stored in this Page
};

// Each data frame starts with a tFrameHead and then followed by read data bytes
struct tFrameHead
{
	uint32_t		magic; // ?, date version of the frame format
	uint32_t 		len; // length of the data bytes in this frame, excluding the head
	uint64_t		nano; // When this frame is filled into the Page
	uint32_t		reserved_; // ?, unused
	char			buf[0]; // the data bytes stored in this Frame
};

#pragma pack()


#endif /* SRC_IO_IOPROTOCOL_H_ */

