/*
 * CSignal.h
 *
 *  Created on: 2018年5月7日
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CSIGNAL_H_
#define LIB_MEMORYQUEUE_CSIGNAL_H_

#include "CRawIOWriter.h"
#include "CRawIOReader.h"
#include "SysConf.h"

constexpr uint32_t 	SIGNAL_PAGE_SIZE = 64 * 1024 * 1024;  // 64M

class CSignalReader : public CRawIOReader
{
public:
	CSignalReader(string name)
	{
		init(string(SIGNAL_BASE_DIR) + name, -1 ,-1);
	}

	string readStr()
	{
		uint32_t len;
		const char* p = read(len);
		if(p) return string(p, len);
		return string();
	}
};

class CSignalWriter : public CRawIOWriter
{
public:
	CSignalWriter(string name) : CRawIOWriter(SIGNAL_PAGE_SIZE)
	{
		init(string(SIGNAL_BASE_DIR) + name);
	}

	bool writeStr(string data)
	{
		return write(data.data(), data.size());
	}
};

#endif /* LIB_MEMORYQUEUE_CSIGNAL_H_ */
