/*
 * CFileReciever.cpp
 *
 *  Created on: 2017年10月2日
 *      Author: hongxu
 */

#include "CFileReciever.h"

#include <string.h>

#include "CWaiter.h"
#include "Logger.h"
#include "CCrypt.h"

CFileReciever::CFileReciever(CWaiter *p_owner) : CCommanderBase(p_owner), is_first_pkg_(true), left_len_(0)
{

}

CFileReciever::~CFileReciever()
{

}

int CFileReciever::run(string &pkg)
{
	if(is_first_pkg_)
	{
		is_first_pkg_ = false;
		return processFirstPkg(pkg);
	}

	return processPkg(pkg);		// wait for left content
}

int CFileReciever::processFirstPkg(string &pkg)
{
	const char *p = pkg.data();
	uint32_t len = pkg.size();

	uint32_t content_len = ((tCommand*)p)->content_len;
	p += sizeof(tCommand);
	len -= sizeof(tCommand);

	if(len < sizeof(tFileHead))
	{
		LOG_INFO("data format err.");
		return STATE_ABORT;
	}

	tFileHead *p_head = (tFileHead*)p;
	file_st_ = p_head->file_st;
	memcpy(digest_, p_head->digest, sizeof(digest_));
	p += sizeof(tFileHead);
	len -= sizeof(tFileHead);

	char *p_null = (char*)memchr(p, '\0', len);
	if(p_null == NULL)
	{
		LOG_INFO("data format err.");
		return STATE_ABORT;
	}

	name_.assign(p, (uint32_t)(p_null - p));
	len -= (uint32_t)(p_null - p) + 1;
	p = p_null + 1;

	if(S_ISDIR(p_head->file_st.st_mode))
	{
		return STATE_FINISH;
	}

	if(len)
	{
		content_.append(p, len);
	}

	if(content_len <= len)
	{
		if(checkDigest()) return STATE_FINISH;
		else
		{
			LOG_INFO("digest mismatch.");
			return STATE_ABORT;
		}
	}

	left_len_ = content_len - len;
	return STATE_PENDING;		// wait for left content
}

int CFileReciever::processPkg(string &pkg)
{
	const char *p = pkg.data();
	uint32_t len = pkg.size();

	uint32_t write_len = len < left_len_ ? len: left_len_;
	content_.append(p, write_len);

	if(left_len_ <= len)		//complete
	{
		if(checkDigest()) return STATE_FINISH;
		else
		{
			LOG_INFO("digest mismatch.");
			return STATE_ABORT;
		}
	}

	left_len_ -= len;
	return STATE_PENDING;		// wait for left content
}

bool CFileReciever::checkDigest()
{
	CCrypt c;
	string d = c.digest(content_);

	return (d.size() == sizeof(digest_) && 0 == memcmp(d.data(), digest_, sizeof(digest_)));
}

