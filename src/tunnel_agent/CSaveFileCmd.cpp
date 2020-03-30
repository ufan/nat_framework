/*
 * CSaveFileCmd.cpp
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#include "CSaveFileCmd.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "CWaiter.h"
#include "protocol.h"
#include "Logger.h"
#include "CCrypt.h"

CSaveFileCmd::CSaveFileCmd(CWaiter *p_owner) : CCommanderBase(p_owner), is_fist_pkg_(true), fd_(-1), left_len_(0)
{

}

CSaveFileCmd::~CSaveFileCmd()
{
	if(fd_ >= 0)
	{
		close(fd_);
	}
}

int CSaveFileCmd::run(string &pkg)
{
	int ret = STATE_ABORT;

	if(is_fist_pkg_)
	{
		is_fist_pkg_ = false;
		ret = processFirstPkg(pkg);
	}
	else
	{
		ret =  processPkg(pkg);		// wait for left content
	}

	if(ret == STATE_FINISH)			p_owner_->sayFinish();
	else if(ret == STATE_ABORT) 	p_owner_->sayFinish(TYPE_FINISH_FAIL);

	return ret;
}

int CSaveFileCmd::processFirstPkg(string &pkg)
{
	const char *p = pkg.data();
	uint32_t len = pkg.size();

	uint32_t content_len = ((tCommand*)p)->content_len;
	p += sizeof(tCommand);
	len -= sizeof(tCommand);

	if(len < sizeof(tHead))
	{
		LOG_INFO("data format err.");
		return STATE_ABORT;
	}

	tHead *p_head = (tHead*)p;
	memcpy(digest_, p_head->digest, sizeof(digest_));
	p += sizeof(tHead);
	len -= sizeof(tHead);

	char *p_null = (char*)memchr(p, '\0', len);
	if(p_null == NULL)
	{
		LOG_INFO("data format err.");
		return STATE_ABORT;
	}

	name_.assign(p, (uint32_t)(p_null - p));
	len -= (uint32_t)(p_null - p) + 1;
	p = p_null + 1;

	if(!S_ISDIR(p_head->file_st.st_mode))
	{
		fd_ = open(name_.c_str(), O_CREAT|O_TRUNC|O_CLOEXEC|O_RDWR, p_head->file_st.st_mode);
		if(fd_ < 0)
		{
			LOG_INFO("create file %s err: %s", name_.c_str(), strerror(errno));
			return STATE_ABORT;
		}
	}
	else
	{
		if(mkdir(name_.c_str(), p_head->file_st.st_mode) < 0)
		{
			LOG_INFO("mkdir %s err: %s", name_.c_str(), strerror(errno));
			return STATE_ABORT;
		}
		return STATE_FINISH;
	}

	if(len)
	{
		int ret = write(fd_, p, len);
		if(ret != len)
		{
			LOG_INFO("write file err: %s", strerror(errno));
			return STATE_ABORT;
		}
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

int CSaveFileCmd::processPkg(string &pkg)
{
	const char *p = pkg.data();
	uint32_t len = pkg.size();

	uint32_t write_len = len < left_len_ ? len: left_len_;
	int ret = write(fd_, p, write_len);
	if(ret != write_len)
	{
		LOG_INFO("write file err: %s", strerror(errno));
		return STATE_ABORT;
	}

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

bool CSaveFileCmd::checkDigest()
{
	fsync(fd_);

	CCrypt c;
	string d = c.digestFile(fd_);

	return (d.size() == sizeof(digest_) && 0 == memcmp(d.data(), digest_, sizeof(digest_)));
}

