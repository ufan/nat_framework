/*
 * CSystemIO.h
 *
 *  Created on: 2018年4月28日
 *      Author: sky
 */

#ifndef IO_CSYSTEMIO_H_
#define IO_CSYSTEMIO_H_

#include "CRawIOReader.h"
#include "CGlobalSafeRawIOWriter.h"
#include "SysConf.h"
#include "utils.h"

class CSystemIO
{
	static const unsigned int MEM_SIZE = 64 * 1024 * 1024; // 64M

	CSystemIO() : sys_writer_(MEM_SIZE)
	{
		sys_writer_.setMemMode(false);
	}

public:
	bool init()
	{
		if(!sys_writer_.hasLoad())
		{
			if(!createPath(IO_SYSTEM_MSG_DIR)) return false;
			return sys_writer_.init(IO_SYSTEM_MSG_PATH);
		}
		return true;
	}

	CGlobalSafeRawIOWriter& getWriter() {return sys_writer_;}

	CRawIOReader* createReader()
	{
		CRawIOReader *sys_reader = new CRawIOReader();
		sys_reader->setMemMode(false);
		sys_reader->init(IO_SYSTEM_MSG_PATH, -1 , -1);
		return sys_reader;
	}

	static CSystemIO& instance();

private:
	CGlobalSafeRawIOWriter		sys_writer_;
};

#endif /* IO_CSYSTEMIO_H_ */
