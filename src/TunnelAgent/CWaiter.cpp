/*
 * CWaiter.cpp
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#include "CWaiter.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <openssl/rand.h>
#include "CExecLoader.h"
#include "CPythonExecutor.h"
#include "CSaveFileCmd.h"
#include "CShellCmd.h"
#include "CStrategyManager.h"
#include "CTunnelAgent.h"
#include "protocol.h"
#include "public_key.h"
#include "Logger.h"


CWaiter::CWaiter(int fd) : fd_(fd), state_(INIT), read_cur_(0), write_cur_(0), is_write_open_(false)
{
	ev_io *p_watcher = tWaiterWatcher::getWatcher(this);
	ev_io_init (p_watcher, readWrite_cb, fd_, EV_READ);
	ev_io_start (ev_default_loop(0), p_watcher);
}

CWaiter::~CWaiter()
{
	ev_io_stop (ev_default_loop(0), tWaiterWatcher::getWatcher(this));
	close(fd_);
}

void CWaiter::readWrite_cb(EV_P_ ev_io* w, int events)
{
	tWaiterWatcher *p_watcher = (tWaiterWatcher *)w;
	p_watcher->waiter.process(EV_A_ w, events);
}

void CWaiter::send(string &s)
{
	if(!is_write_open_)
	{
		struct ev_loop *loop = ev_default_loop(0);
		ev_io *p_watcher = tWaiterWatcher::getWatcher(this);
		ev_io_stop (loop, p_watcher);
		ev_io_init (p_watcher, readWrite_cb, fd_, EV_READ | EV_WRITE);
		ev_io_start (loop, p_watcher);
		is_write_open_ = true;
	}

	write_buf_ += s;
}

void CWaiter::resetListen(int events)
{
	struct ev_loop *loop = ev_default_loop(0);
	ev_io *p_watcher = tWaiterWatcher::getWatcher(this);
	ev_io_stop (loop, p_watcher);
	ev_io_init (p_watcher, readWrite_cb, fd_, events);
	ev_io_start (loop, p_watcher);

	is_write_open_ = (events & EV_WRITE) ? true : false;
}

void CWaiter::handleConnectionBroken()
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(fd_, (struct sockaddr *)&addr, &addrlen);
	LOG_INFO("%d|connection [would] broken, peer %s:%d", fd_, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

bool CWaiter::write_cb()
{
	if(write_buf_.empty()) return true;

	int ret = write(fd_, (void*)(write_buf_.data() + write_cur_), write_buf_.size() - write_cur_);
	if(ret == 0)
	{
		handleConnectionBroken();
		return false;
	}
	else if(ret < 0)
	{
		if(errno != EAGAIN)
		{
			LOG_ERR("%d|write err:%d", fd_, errno);
			handleConnectionBroken();
			return false;
		}
	}
	else
	{
		write_cur_ += ret;
	}

	if(write_cur_ == write_buf_.size())
	{
		write_buf_.clear();
		write_cur_ = 0;
		resetListen(EV_READ);
	}

	return true;
}

int CWaiter::read_cb()
{
	if(read_buf_.empty())
	{
		read_cur_ = 0;
		read_buf_.resize(sizeof(tMetaHead));
	}

	while(read_cur_ < read_buf_.size())
	{
		int ret = read(fd_, (void*)(read_buf_.data() + read_cur_), read_buf_.size() - read_cur_);
		if(ret == 0)
		{
			handleConnectionBroken();
			return STATE_ABORT;
		}
		else if(ret < 0)
		{
			if(errno != EAGAIN)
			{
				LOG_ERR("%d|read err:%d", fd_, errno);
				handleConnectionBroken();
				return STATE_ABORT;
			}
			return STATE_PENDING;
		}

		read_cur_ += ret;

		if(read_cur_ == sizeof(tMetaHead))			// got head
		{
			tMetaHead *p_head = (tMetaHead *)read_buf_.data();
			if(p_head->stx != META_HEAD_STX || p_head->ver != META_HEAD_VER
					|| p_head->len > MAX_DATA_LEN || p_head->len == 0)
			{
				LOG_INFO("%d|got wrong format data.", fd_);
				handleConnectionBroken();
				return STATE_ABORT;
			}

			read_buf_.resize(p_head->len + sizeof(tMetaHead));
		}
		else if(read_cur_ == read_buf_.size())		// pkg complete
		{
			return STATE_NEXT;
		}
	}
	return true;
}

int CWaiter::process(EV_P_ ev_io *w, int events)
{
	int ret = STATE_PENDING;
	if(events & EV_WRITE)		// send some data
	{
		if(!write_cb())		// socket err occurs
		{
			ret = STATE_ABORT;
		}
	}

	if((events & EV_READ) && ret != STATE_ABORT)
	{
		ret = read_cb();
		if(STATE_NEXT == ret)	// pkg driven
		{
			ret = processPkg();
			read_buf_.clear();	// pkg has been processed, clear it. must do it.
		}
	}

	if(STATE_ABORT == ret || STATE_FINISH == ret)
	{
		if(STATE_ABORT == ret)	LOG_INFO("%d|waiter abort.", fd_);
		else LOG_INFO("%d|waiter finish.", fd_);

		CTunnelAgent::instance()->releaseWaiter(fd_);
	}

	return ret;
}

int CWaiter::processPkg()
{
	string pkg;
	if(state_ > GET_DES_KEY)
	{
		string enstr = read_buf_.substr(sizeof(tMetaHead));
		pkg = crypter_.desDecrypt(enstr);
	}

	int ret = STATE_PENDING;
	do
	{
		switch(state_)
		{
		case INIT:
			ret = init();
			break;

		case SSL_AUTH:
			ret = processSSLAuth(read_buf_);
			break;

		case GET_DES_KEY:
			ret = get3DESkey(read_buf_);
			break;

		case DO_COMMAND:
			ret = doCommand(pkg);
			break;

		case FINISH:
			ret = STATE_FINISH;
			break;

		default:
			LOG_ERR("bug");
			return STATE_ABORT;
		}
	}while(STATE_NEXT == ret);
	return ret;
}

int CWaiter::init()
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	getpeername(fd_, (struct sockaddr *)&addr, &addrlen);
	LOG_INFO("%d|waiter init for peer %s:%d", fd_, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	state_ = SSL_AUTH;
	return STATE_NEXT;
}

int CWaiter::processSSLAuth(string &pkg)
{
	CCrypt 		crypter;
	crypter.setRSAPublicKey(g_public_key);

	tSSLAuthenBody *p_body = (tSSLAuthenBody*)(pkg.data() + sizeof(tMetaHead));
	string encrypted((const char*)p_body->encrypted, pkg.size() - sizeof(tMetaHead) - sizeof(tSSLAuthenBody));
	string decrypt = crypter.rsaPublicDecrypt(encrypted);

	if(decrypt.size() != sizeof(p_body->rand)
			|| 0 != memcmp(decrypt.data(), p_body->rand, sizeof(p_body->rand)))
	{
		LOG_INFO("%d|authentication failed.", fd_);
		return STATE_ABORT;
	}

	tSSLAuthenBody body;
	RAND_bytes((unsigned char*)body.rand, sizeof(body.rand));

	string randbyte(body.rand, sizeof(body.rand));
	string encrypt = crypter.rsaPublicEncrypt(randbyte);

	tMetaHead head = {META_HEAD_STX, META_HEAD_VER, 0,
		(uint32_t)(randbyte.size() + encrypt.size())};

	string s((const char*)&head, sizeof(head));
	s += randbyte;
	s += encrypt;

	send(s);

	state_ = GET_DES_KEY;
	return STATE_PENDING;
}

int CWaiter::get3DESkey(string &pkg)
{
	CCrypt 		crypter;
	crypter.setRSAPublicKey(g_public_key);

	string en = pkg.substr(sizeof(tMetaHead));
	string decrypt = crypter.rsaPublicDecrypt(en);

	if(decrypt.size() != KEY_SIZE_OF_3DES)
	{
		LOG_INFO("%d|data format err.", fd_);
		return STATE_ABORT;
	}

	crypter_.set3DesKey(decrypt.data());
	state_ = DO_COMMAND;
	return STATE_PENDING;
}

int CWaiter::doCommand(string &pkg)
{
	if(!p_commander_)
	{
		tCommand *p_head = (tCommand*)pkg.data();
		switch(p_head->cmd)
		{
		case CMD_SAVEFILE:
			p_commander_.reset(new CSaveFileCmd(this));
			break;

		case CMD_SHELL:
			p_commander_.reset(new CShellCmd(this));
			break;

		case CMD_PYTHON:
			p_commander_.reset(new CPythonExecutor(this));
			break;

		case CMD_EXEC:
			p_commander_.reset(new CExecLoader(this));
			break;

		case CMD_STG:
			p_commander_.reset(new CStrategyManager(this));
			break;

		case CMD_BYE:
			state_ = FINISH;
			return STATE_NEXT;

		default:
			LOG_INFO("%d|Unknown cmd.", fd_);
			return STATE_ABORT;
		}
	}

	if(p_commander_)
	{
		int ret = p_commander_->run(pkg);
		if(STATE_FINISH == ret)
		{
			p_commander_.reset();
			return STATE_PENDING;
		}
		else if(STATE_NEXT == ret)
		{
			p_commander_.reset();
		}
		return ret;
	}

	LOG_ERR("should not get here");
	return STATE_ABORT;
}

void CWaiter::desSendData(string &s)
{
	string en = crypter_.desEncrypt(s);
	tMetaHead head = {META_HEAD_STX, META_HEAD_VER, 0,
		uint32_t(en.size())};

	string buf((const char *)&head, sizeof(head));
	buf += en;
	send(buf);
}

void CWaiter::sayFinish(uint8_t type)
{
	tCommand cmd_head = {CMD_FINISH, type, 0};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	desSendData(data);
}

