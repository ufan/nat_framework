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

/*
 * CSystemIO is an singlton in charge of the System IO pages.
 * It's aimed to be used as communication channel in multi-process
 * and multi-threading environment.
 */

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

  // Get a reader for reading the IO Page data
	CRawIOReader* createReader()
	{
		CRawIOReader *sys_reader = new CRawIOReader();
		sys_reader->setMemMode(false);

    // start from the fisrt frame of the first Page file
		sys_reader->init(IO_SYSTEM_MSG_PATH, -1 , -1);
		return sys_reader;
	}

	static CSystemIO& instance();

private:
	CGlobalSafeRawIOWriter		sys_writer_;
};

#endif /* IO_CSYSTEMIO_H_ */
