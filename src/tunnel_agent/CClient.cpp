/*
 * CClient.cpp
 *
 *  Created on: 2017年9月27日
 *      Author: hongxu
 */

#include "CClient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <openssl/rand.h>
#include "private_key.h"
#include "protocol.h"
#include "utils.h"
#include "strategy_shared_comm.h"
#include "json.hpp"
using json = nlohmann::json;


CConfig* CClient::getConfig()
{
	static CConfig s_config;
	return &s_config;
}

CClient::CClient() : fd_(-1)
{

}

CClient::~CClient()
{
	if(fd_ >= 0)
	{
		close(fd_);
		fd_ = -1;
	}
}

bool CClient::init(string cfg)
{
	// init configure class
	CConfig* p_config = getConfig();
	ASSERT_RET(p_config->init(cfg), false);

	// init logger
	string logcfg = p_config->getVal<string>("COMMON", "client_log", string());
	ASSERT_RET(initLogger(logcfg), false);

	string ip = p_config->getVal<string>("AGENT", "ip");
	uint16_t port = p_config->getVal<uint16_t>("AGENT", "port");

	ASSERT_RET(connect(ip, port), false);

	return true;
}

bool CClient::connect(string ip, uint16_t port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(ip.c_str(), &addr.sin_addr);

    if(-1 == (fd_ = socket(AF_INET, SOCK_STREAM, 0)))
    {
        LOG_ERR("error in create socket:%s", strerror(errno));
        return false;
    }

    if(-1 == ::connect(fd_, (struct sockaddr*)&addr, sizeof(struct sockaddr)))
    {
        LOG_ERR("connect error:%s", strerror(errno));
        close(fd_);
        fd_ = -1;
        return false;
    }

	return true;
}

bool CClient::sslAuthen()
{
	tSSLAuthenBody body;
	RAND_bytes((unsigned char*)body.rand, sizeof(body.rand));

	crypter_.setRSAPrivateKey(g_private_key);

	// authen myself
	string randbyte(body.rand, sizeof(body.rand));
	string encrypt = crypter_.rsaPrivateEncrypt(randbyte);

	tMetaHead head = {META_HEAD_STX, META_HEAD_VER, 0,
		uint32_t(randbyte.size() + encrypt.size())};

	string buf((const char *)&head, sizeof(head));
	buf += randbyte;
	buf += encrypt;

	int ret = write(fd_, buf.data(), buf.size());
	if(ret < buf.size())
	{
		LOG_ERR("write data err:%s", strerror(errno));
		return false;
	}

	// verify server
	memset((void*)&head, 0, sizeof(head));
	ret = read(fd_, (void*)&head, sizeof(head));
	if(ret <= 0)
	{
		LOG_ERR("read err:%s", strerror(errno));
		return false;
	}

	if(head.stx != META_HEAD_STX || head.ver != META_HEAD_VER
			|| head.len > MAX_DATA_LEN || head.len == 0)
	{
		LOG_ERR("bad format data.");
		return false;
	}

	buf.resize(head.len);
	ret = read(fd_, (void*)buf.data(), head.len);
	if(ret < head.len)
	{
		LOG_ERR("read err:%s", strerror(errno));
		return false;
	}

	tSSLAuthenBody *p_body = (tSSLAuthenBody*)buf.data();
	string encrypted = buf.substr(offsetof(tSSLAuthenBody, encrypted));
	string decrypt = crypter_.rsaPrivateDecrypt(encrypted);

	if(decrypt.size() != sizeof(p_body->rand)
			|| 0 != memcmp(decrypt.data(), p_body->rand, sizeof(p_body->rand)))
	{
		LOG_ERR("server authentication failed.");
		return false;
	}

	return true;
}

bool CClient::authentication()
{
	if(!sslAuthen()) return false;

	return true;
}

bool CClient::sendDesKey()
{
	crypter_.gen3DesKey();

	string source(crypter_.get3DesKeyBlock(), KEY_SIZE_OF_3DES);
	string encrypt = crypter_.rsaPrivateEncrypt(source);

	tMetaHead head = {META_HEAD_STX, META_HEAD_VER, 0,
		uint32_t(encrypt.size())};

	string buf((const char *)&head, sizeof(head));
	buf += encrypt;

	int ret = write(fd_, buf.data(), buf.size());
	if(ret < buf.size())
	{
		LOG_ERR("write data err:%s", strerror(errno));
		return false;
	}

	return true;
}

bool CClient::desSend(string pkg)
{
	string en = crypter_.desEncrypt(pkg);
	tMetaHead head = {META_HEAD_STX, META_HEAD_VER, 0,
		uint32_t(en.size())};

	string buf((const char *)&head, sizeof(head));
	buf += en;

	int ret = write(fd_, buf.data(), buf.size());
	if(ret < buf.size())
	{
		LOG_ERR("write data err:%s", strerror(errno));
		return false;
	}

	return true;
}

bool CClient::doCommand(vector<string> &extr_cmd)
{
	CConfig* p_config = getConfig();

	vector<string> cmds;
	p_config->getValList("TASK", "cmd_list", cmds);

	for(uint32_t i = 0; i < extr_cmd.size(); ++i)
	{
		cmds.push_back(extr_cmd[i]);
	}

	for(vector<string>::iterator iter = cmds.begin(); iter != cmds.end(); ++iter)
	{
		LOG_INFO("%s", iter->c_str());
		if(!doOneCommand(*iter))
		{
			LOG_ERR("command\t\t" SERR("[FAIL]"));
			return false;
		}
		else
		{
			LOG_INFO("command\t\t" SOK("[SUCC]"));
		}
	}
	return true;
}

bool CClient::doOneCommand(string cmd)
{
	vector<string> cmd_split;
	splitCmdLine(cmd, cmd_split);

	string order = cmd_split[0];
	if(strcasecmp(order.c_str(), "SAVE") == 0)
	{
		if(cmd_split.size() != 3) return false;
		return saveFile(cmd_split[1], cmd_split[2]);
	}
	else if(strcasecmp(order.c_str(), "SHELL") == 0)
	{
		return doShellCmd(joinCmdVector(cmd_split, " ", 1));
	}
	else if(strcasecmp(order.c_str(), "ASSERT") == 0)
	{
		if(cmd_split.size() != 2) return false;
		return doAssertCmd(cmd_split[1]);
	}
	else if(strcasecmp(order.c_str(), "EXEC") == 0)
	{
		if(cmd_split.size() < 3) return false; // EXEC config exe args
		return doExecCmd(cmd_split[1], cmd_split[2], joinCmdVector(cmd_split, " ", 3));
	}
	else if(strcasecmp(order.c_str(), "EXEC_DIR") == 0)
	{
		if(cmd_split.size() < 4) return false;   // EXEC_DIR workdir config exe args
		return doExecCmd(cmd_split[2], cmd_split[3], joinCmdVector(cmd_split, " ", 4), cmd_split[1]);
	}
	else if(strcasecmp(order.c_str(), "STG") == 0)
	{
		return doStgCmd(cmd_split);
	}
	else return false;
	return true;
}

void CClient::run(vector<string> &extr_cmd)
{
	do
	{
		if(!authentication()) break;
		LOG_INFO("Authentication succ.");

		if(!sendDesKey()) break;

		if(!doCommand(extr_cmd)) break;

	}while(0);
	sayBye();
}

bool CClient::loopSendFile(int fd, string &data)
{
#define BLOCK_SIZE 1024
	uint32_t off = data.size();
	data.resize(off + BLOCK_SIZE);
	int ret = read(fd, (char*)(data.data() + off), sizeof(BLOCK_SIZE));
	if(ret < 0)
	{
		LOG_ERR("read file err: %s", strerror(errno));
		return false;
	}
	else data.resize(off + ret);

	if(!desSend(data)) return false;

	data.resize(BLOCK_SIZE);
	while(0 < (ret = read(fd, (char*)data.data(), BLOCK_SIZE)))
	{
		data.resize(ret);
		if(!desSend(data)) return false;
		data.resize(BLOCK_SIZE);
	}
	return true;
#undef BLOCK_SIZE
}

bool CClient::saveFile(string file, string dst)
{
	if(file.size() >= 2 && (file[0] == '\'' || file[0] == '"'))
		file = file.substr(1, file.size() - 2);
	if(dst.size() >= 2 && (dst[0] == '\'' || dst[0] == '"'))
		dst = dst.substr(1, dst.size() - 2);

	int fd = open(file.c_str(), O_CLOEXEC|O_RDONLY);
	if(fd < 0)
	{
		LOG_ERR("cannot open file %s err: %s", file.c_str(), strerror(errno));
		return false;
	}

	struct stat stbuf;
	if(fstat(fd, &stbuf) < 0)
	{
		LOG_ERR("stat file %s err: %s", file.c_str(), strerror(errno));
		close(fd);
		return false;
	}

	tCommand cmd_head = {CMD_SAVEFILE, 0, (uint32_t)stbuf.st_size};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	if(!S_ISDIR(stbuf.st_mode))
	{
		data += crypter_.digestFile(fd);
		data += string((char*)&stbuf, sizeof(stbuf));
		data += dst;
		data += '\0';

		bool result = loopSendFile(fd, data);
		close(fd);
		if(!result) return false;
	}
	else
	{
		close(fd);
		data.resize(data.size() + 32);
		data += string((char*)&stbuf, sizeof(stbuf));
		data += dst;
		data += '\0';
		if(!desSend(data)) return false;
	}
	return loopReadAndEcho();
}

bool CClient::sayBye()
{
	tCommand cmd_head = {CMD_BYE, 0, 0};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	return desSend(data);
}

bool CClient::desRead(string &pkg)
{
	tMetaHead head;
	string buf;

	int ret = read(fd_, (void*)&head, sizeof(head));
	if(ret <= 0)
	{
		if(ret == 0) LOG_ERR("read err:closed by peer.");
		else LOG_ERR("read err:%s", strerror(errno));
		return false;
	}

	if(head.stx != META_HEAD_STX || head.ver != META_HEAD_VER
						|| head.len > MAX_DATA_LEN || head.len == 0)
	{
		LOG_ERR("got wrong format data.");
		return false;
	}

	buf.resize(head.len);
	ret = read(fd_, (void*)buf.data(), head.len);
	if(ret < head.len)
	{
		LOG_ERR("read err:%s", strerror(errno));
		return false;
	}

	pkg = crypter_.desDecrypt(buf);
	return true;
}

bool CClient::doShellCmd(string cmd)
{
	last_shell_output_.clear();
	tCommand cmd_head = {CMD_SHELL, 0, (uint32_t)cmd.size()};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	data += cmd;
	if(!desSend(data)) return false;

	while(desRead(data))
	{
		if(data[0] == CMD_ALLDATA)
		{
			cout << data.c_str() + 1;
			last_shell_output_ += data.substr(1);
		}
		else if(data[0] == CMD_FINISH)
		{
			tCommand *p = (tCommand*)data.data();
			return p->type == TYPE_FINISH_SUCC ? true : false;
		}
		else
		{
			cout << endl;
			LOG_ERR("client read data format err.");
			return false;
		}
	}
	return false;
}

bool CClient::doAssertCmd(string target)
{
	if(target.size() >= 2 && (target[0] == '\'' || target[0] == '"'))
		target = target.substr(1, target.size() - 2);

	target.push_back('\n');
	return last_shell_output_ == target;
}

bool CClient::loopReadAndEcho()
{
	string data;
	while(desRead(data))
	{
		if(data[0] == CMD_ALLDATA)
		{
			cout << data.c_str() + 1;
		}
		else if(data[0] == CMD_FINISH)
		{
			tCommand *p = (tCommand*)data.data();
			return p->type == TYPE_FINISH_SUCC ? true : false;
		}
		else
		{
			LOG_ERR("client read data format err.");
			return false;
		}
	}
	return false;
}

int CClient::readEcho()
{
	string data;
	while(desRead(data))
	{
		if(data[0] == CMD_ALLDATA)
		{
			cout << data.c_str() + 1;
		}
		else if(data[0] == CMD_FINISH)
		{
			tCommand *p = (tCommand*)data.data();
			return p->type;
		}
		else
		{
			LOG_ERR("client read data format err.");
			return TYPE_FINISH_FAIL;
		}
	}
	return TYPE_FINISH_FAIL;
}

bool CClient::sendFile(string file, string name, uint8_t cmd, uint8_t type)
{
	if(file.size() >= 2 && (file[0] == '\'' || file[0] == '"'))
		file = file.substr(1, file.size() - 2);

	int fd = open(file.c_str(), O_CLOEXEC|O_RDONLY);
	if(fd < 0)
	{
		LOG_ERR("cannot open file %s err: %s", file.c_str(), strerror(errno));
		return false;
	}

	struct stat stbuf;
	if(fstat(fd, &stbuf) < 0)
	{
		LOG_ERR("stat file %s err: %s", file.c_str(), strerror(errno));
		close(fd);
		return false;
	}

	tCommand cmd_head = {cmd, type, (uint32_t)stbuf.st_size};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	if(!S_ISDIR(stbuf.st_mode))
	{
		data += crypter_.digestFile(fd);
		data += string((char*)&stbuf, sizeof(stbuf));
		data += name;
		data += '\0';

		bool result = loopSendFile(fd, data);
		close(fd);
		return result;
	}

	close(fd);
	data.resize(data.size() + 32);
	data += string((char*)&stbuf, sizeof(stbuf));
	data += name;
	data += '\0';
	return desSend(data);
}

bool CClient::sendDir(string localdir, string remotedir, uint8_t cmd, uint8_t type)
{
	tCommand cmd_head = {cmd, type, 0};
	string data((const char*)&cmd_head, sizeof(cmd_head));
	data += remotedir;
	if(!desSend(data)) return false;			// send remote base dirname
	if(remotedir.empty()) return true;		// if remote dirname is empty, just return true.

	// loop send file to remote dir
	vector<string> pathlist;
	pathlist.push_back("/");
	while(pathlist.size())
	{
		string path = pathlist.back();
		pathlist.pop_back();

		string localpath = localdir + path;
	    DIR * dir = opendir(localpath.c_str());
	    if(NULL == dir)
	    {
	    		LOG_ERR("open %s err: %s", localpath.c_str(), strerror(errno));
	    		return false;
	    }

	    struct dirent * dirfile = NULL;
	    while((dirfile = readdir(dir)) != NULL)
	    {
	        string name = dirfile->d_name;
	        if(name == "." || name == "..") continue;
	        name = path + "/" + name;
	        if(!sendFile(localdir + name, name, cmd, type)) return false;
	        if(dirfile->d_type & DT_DIR) pathlist.push_back(name);
	    }
	    closedir(dir);
	}

	cmd_head.cmd = CMD_FINISH;
	data.assign((const char*)&cmd_head, sizeof(cmd_head));
	return desSend(data);
}

bool CClient::doExecCmd(string config, string execfile, string argstr, string workdir)
{
	ifstream in(config);
	if(!in)
	{
		LOG_ERR("read config file %s err.\n", config.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	string name;
	try
	{
		json j = json::parse(content);
		name = j["name"];
	}catch(...)
	{
		LOG_ERR("parse config file %s err.\n", config.c_str());
		return false;
	}

	bool result = sendFile(execfile, name, CMD_EXEC);
	if(!result || readEcho() != TYPE_FINISH_ACK) return false;

	result = sendFile(config, name, CMD_EXEC);
	if(!result) return false;
	int echo = readEcho();
	if(echo == TYPE_FINISH_SUCC) return true;
	else if(echo == TYPE_FINISH_FAIL) return false;

	if(workdir.empty())
	{
		result = sendDir(workdir, workdir, CMD_EXEC);
	}
	else
	{
		string tmp = workdir;
		char *base = basename((char*)tmp.c_str());
		char buf[256];
		snprintf(buf, sizeof buf, "$TMP/%s_%lu", base, (unsigned long)time(NULL));
		result = sendDir(workdir, buf, CMD_EXEC);
	}
	if(!result || readEcho() != TYPE_FINISH_ACK) return false;
	result = desSend(argstr);
	if(!result) return false;
	return readEcho() == TYPE_FINISH_SUCC;
}

bool CClient::doStgCmd(vector<string> cmd)
{
	if(cmd.size() < 2)
	{
		LOG_ERR("command syntax err.");
		return false;
	}
	string type = cmd[1];

	string name;
	tStgCommand cmd_head = {CMD_STG, 0, 0};
	if(strcasecmp(type.c_str(), "status") == 0)
	{
		cmd_head.type = STG_STATUS;
	}
	else if(strcasecmp(type.c_str(), "start") == 0)
	{
		if(cmd.size() < 3)
		{
			LOG_ERR("syntax: start name");
			return false;
		}
		cmd_head.type = STG_START_TRADE;
		name = cmd[2];
	}
	else if(strcasecmp(type.c_str(), "stop") == 0)
	{
		if(cmd.size() < 3)
		{
			LOG_ERR("syntax: stop name");
			return false;
		}
		cmd_head.type = STG_STOP_TRADE;
		name = cmd[2];
	}
	else if(strcasecmp(type.c_str(), "exit") == 0)
	{
		if(cmd.size() < 3)
		{
			LOG_ERR("syntax: exit name");
			return false;
		}
		cmd_head.type = STG_EXIT;
		name = cmd[2];
	}
	else if(strcasecmp(type.c_str(), "signal") == 0)
	{
		if(cmd.size() < 4)
		{
			LOG_ERR("syntax: signal name sig [data]");
			return false;
		}
		cmd_head.type = STG_SIGNAL;
		cmd_head.data = (uint32_t)strtol(cmd[3].c_str(), NULL, 0);
		name = cmd[2];

		if(cmd.size() > 4)
		{
			uint64_t data = strtoul(cmd[4].c_str(), NULL, 0);
			cmd_head.data |= (data << 32);
		}
	}
	else if(strcasecmp(type.c_str(), "set") == 0)
	{
		if(cmd.size() < 4)
		{
			LOG_ERR("syntax: set data name");
			return false;
		}
		cmd_head.type = STG_SET_DATA;
		cmd_head.data = strtoull(cmd[2].c_str(), NULL, 0);
		name = cmd[3];
	}
	else
	{
		LOG_ERR("command syntax err.");
		return false;
	}

	string data((const char*)&cmd_head, sizeof(cmd_head));
	if(!name.empty()) data += name;
	if(!desSend(data)) return false;
	return loopReadAndEcho();
}

