/*
 * CGlobalSafeRawIOWriter.cpp
 *
 *  Created on: 2018年5月9日
 *      Author: hongxu
 */

#include "CGlobalSafeRawIOWriter.h"
#include "MurmurHash2.h"

CGlobalLock CGlobalSafeRawIOWriter::s_safe_io_lock_;

bool CGlobalSafeRawIOWriter::init(string path)
{
	if(!s_safe_io_lock_.isWork() && !s_safe_io_lock_.init(GLOBAL_SAFE_RAW_IO_LOCK_FILE, 4096 * 16))
	{
		LOG_ERR("global locker init err.");
		return false;
	}

	hash_ = MurmurHash2(path.c_str(), path.size(), 0x20180426);

	s_safe_io_lock_.lock(hash_);
	bool ret = CRawIOWriter::init(path);
	s_safe_io_lock_.unlock(hash_);

	return ret;
}

bool CGlobalSafeRawIOWriter::write(const void* data, uint32_t len)
{
	s_safe_io_lock_.lock(hash_);
	bool ret = CRawIOWriter::write(data, len);
	s_safe_io_lock_.unlock(hash_);

	return ret;
}

char* CGlobalSafeRawIOWriter::prefetch(uint32_t len)
{
	s_safe_io_lock_.lock(hash_);
	char* ret = CRawIOWriter::prefetch(len);
	return ret;
}

void CGlobalSafeRawIOWriter::commit()
{
	CRawIOWriter::commit();
	s_safe_io_lock_.unlock(hash_);
}

void CGlobalSafeRawIOWriter::discard()
{
	s_safe_io_lock_.unlock(hash_);
}

