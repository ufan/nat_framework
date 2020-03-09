/*
 * CSignalServer.cpp
 *
 *  Created on: Jun 25, 2018
 *      Author: hongxu
 */

#include "CSignalServer.h"
#include "SignalAgentProtocol.h"
#include "SysConf.h"

extern long g_system_time_ns;

CSignalServer::CSignalServer(int fd, long timeout, uint32_t buf_size) : CService(4096, buf_size), timeout_(timeout)
{
	setFd(fd);
	setHeadSize(sizeof(tSACmdHead));
	lst_alive_time_ = g_system_time_ns + timeout_;
}

int CSignalServer::processHead(const char *p, uint32_t len)
{
	tSACmdHead *p_head = (tSACmdHead*)p;
	if(!CHECK_SA_HEAD(p_head) || !setPkgSize(p_head->len))
	{
		LOG_INFO("bad data");
		return -1;
	}
	return 0;
}

int CSignalServer::processPkg(const char *p, uint32_t len)
{
	lst_alive_time_ = g_system_time_ns;
	tSACmdHead *p_head = (tSACmdHead*)p;
	switch(p_head->cmd)
	{
	case SACMD_REQ_SIG:
	{
		const char *p_name = ((tSARequestSignal*)p)->sig_names;
		for(int i = 0; i < ((tSARequestSignal*)p)->sig_cnt; ++i)
		{
			string sig_name = p_name;
			addReader(sig_name);
			p_name += sig_name.size() + 1;
		}
		break;
	}
	case SACMD_REQ_HEATBEAT:
	{
		tSACmdHead head;
		head.cmd = SACMD_REP_HEATBEAT;
		sendData((const char*)&head, sizeof(head));
		break;
	}
	default:
		LOG_INFO("unknown cmd %d", p_head->cmd);
		return -1;
	}
	return 0;
}

void CSignalServer::addReader(string sig_name)
{
	if(read_signal_set_.count(sig_name) == 0)
	{
		readers_.emplace_back(new CSignalReader(sig_name));
		read_signal_set_.insert(sig_name);
		LOG_INFO("%p:%d set io readeer %s", this, (int)(readers_.size()-1), sig_name.c_str());
	}
	else
	{
		LOG_INFO("signal %s has already subscribed.", sig_name.c_str());
	}
}

void CSignalServer::tryRead(bool scan_unload)
{
	tSASignalData data;
	data.cmd = SACMD_SIGNAL_DATA;

	int cnt = readers_.size();
	for(int i = 0; i < cnt; i++)
	{
		auto &r = readers_[i];
		if(!r->hasLoad() && !scan_unload) continue;

		data.sig_idx = i;
		const tFrameHead* frame = r->getCurFrame();
		while(frame)
		{
			uint32_t data_size = sizeof(data) + frame->len;
			if(write_buf_.left() >= data_size)
			{
				data.len = sizeof(data) + frame->len;
				sendData((const char*)&data, sizeof(data));
				sendData(frame->buf, frame->len);
				r->passFrame();
				frame = r->getCurFrame();
			}
			else if(write_buf_.size_ < data_size)
			{
				LOG_ERR("%p:%d skip data, size %lu too big.", this, i, (unsigned long)data_size);
				r->passFrame();
				frame = r->getCurFrame();
			}
			else break;
		}
	}
}

bool CSignalServer::isOK()
{
	return fd_ >= 0 && (lst_alive_time_ + timeout_ > g_system_time_ns);
}
