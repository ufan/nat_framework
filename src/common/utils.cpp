/*
 * utils.cpp
 *
 *  Created on: 2017年10月1日
 *      Author: hongxu
 */

#include <string>
#include <vector>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sched.h>
#include "utils.h"
#include "Logger.h"

using namespace std;

extern char **environ;

void setDaemon()
{
    pid_t pid;
    if ((pid = fork()) != 0)	// father process exit
    {
        exit(0);
    }

    setsid();					// be session leader

    if ((pid = fork()) != 0)	// lose leader, then lose terminal controller
    {
        exit(0);
    }

    //chdir("/");
    umask(0);

    for(int i = 0; i < 1024; i++)
    {
    	close(i);
    }
}

void splitCmdLine(string cmd, vector<string> &res)
{
	char left_bracket = '\0';
	string para;
	const char *p = cmd.c_str();
	while(*p != '\0')
	{
		if(left_bracket != '\0')
		{
			while(*p != '\0')
			{
				if(*p == '\\')
				{
					para.push_back(*p);
					if(*++p == '\0') break;
					para.push_back(*p++);
				}
				else if(*p == left_bracket)
				{
					left_bracket = '\0';
					para.push_back(*p++);
					break;
				}
				else para.push_back(*p++);
			}
		}
		else if(*p == '"' || *p == '\'')
		{
			left_bracket = *p;
			para.push_back(*p++);
		}
		else if(isblank(*p))
		{
			if(!para.empty())
			{
				res.push_back(para);
				para.clear();
			}
			while(isblank(*p) && *++p != '\0');
		}
		else
		{
			para.push_back(*p++);
		}
	}
	if(!para.empty())
	{
		res.push_back(para);
	}
	return;
}

string joinCmdVector(vector<string> cmd, string delimiter, uint32_t start_pos)
{
	if(start_pos >= cmd.size()) return string();
	string res = cmd[start_pos];
	for(uint32_t i = start_pos + 1; i < cmd.size(); i++)
	{
		res += delimiter;
		res += cmd[i];
	}
	return res;
}

void setProcTitle(string title, char **p_os_argv)
{
	prctl(PR_SET_NAME, title.c_str());

	uint32_t size = 0;
	for(int i = 0; environ[i]; i++)
	{
		size += strlen(environ[i]) + 1;
	}

	char *p = new char[size];
	char *argv_last = p_os_argv[0];
	for(int i = 0; p_os_argv[i]; i++)
	{
		if(argv_last == p_os_argv[i])
		{
			argv_last = p_os_argv[i] + strlen(p_os_argv[i]) + 1;
		}
	}

	for(int i = 0; environ[i]; i++)
	{
		if(argv_last == environ[i])
		{
			size = strlen(environ[i]) + 1;
			argv_last = environ[i] + size;
			strcpy(p, environ[i]);
			environ[i] = p;
			p += size;
		}
	}
	argv_last--;
	p_os_argv[1] = NULL;
	strncpy(p_os_argv[0], title.c_str(), argv_last - p_os_argv[0]);
}

bool setProcTitle(string titile)
{
	pid_t pid = getpid();
	char buf[2048];
	sprintf(buf, "/proc/%d/cmdline", pid);
	int fd = open(buf, O_CLOEXEC|O_RDONLY);
	if(fd < 0)
	{
		perror("open cmdline file err");
		return false;
	}
	int ret = read(fd, buf, sizeof(buf));
	close(fd);
	if(ret <= 0) return false;
	buf[sizeof(buf) - 1] = '\0';

	char *p = environ[0] - ret;
	if(memcmp(p, buf, ret) != 0) return false;
	int i = 0;
	for(int j = 0; j < ret; j++)
	{
		if(p[j] == '\0') i++;
	}
	char **argv = new char*[i+1];
	for(int j = 0; j < i; j++)
	{
		argv[j] = p;
		p += strlen(p) + 1;
	}
	argv[i] = NULL;
	setProcTitle(titile, argv);
	delete [] argv;
	return true;
}

void findAllProcessByCmdLine(string cmdline, vector<pid_t> &res)
{
	struct dirent * dirfile;
	DIR * dir = opendir("/proc");
	if(NULL == dir)
	{
		LOG_ERR("open /proc err: %d", errno);
		return;
	}

	char buf[256];
	while((dirfile = readdir(dir)) != NULL)
	{
		if(!(dirfile->d_type & DT_DIR)) continue;

		const char *p = dirfile->d_name;
		while(*p && isdigit(*p)) p++;
		if(*p) continue;		// filter for pid

		sprintf(buf, "/proc/%s/cmdline", dirfile->d_name);

		int fd = open(buf, O_CLOEXEC|O_RDONLY);
		if(fd < 0) continue;
		int ret = read(fd, buf, sizeof(buf));
		close(fd);
		if(ret <= 0) continue;

		buf[sizeof(buf) - 1] = '\0';
		if(cmdline == buf)
		{
			pid_t pid = strtol(dirfile->d_name, NULL, 0);
			res.push_back(pid);
		}
	}

	closedir(dir);
	return;
}

pid_t findProcessByCmdLine(string cmdline)
{
	vector<pid_t> res;
	findAllProcessByCmdLine(cmdline, res);
	if(res.empty()) return -1;
	return res[0];
}

bool setaffinity(int cpuid)
{
    cpu_set_t mask;
    cpu_set_t get;

    CPU_ZERO(&mask);
    CPU_SET(cpuid,&mask);

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
    {
    	perror("setaffinity err");
    	return false;
    }
    return true;
}

bool setRealtimeSchedPolicy(int prior)
{
	sched_param param;
	param.sched_priority = prior;

	if(0 != sched_setscheduler(getpid(), SCHED_FIFO, &param))
	{
		perror("set realtime schedule err");
		return false;
	}
	return true;
}

bool createDirTree(string path)
{
	int pos = 0;
	int len = path.size();
	while(pos < len && path[pos] == '/') pos++;
	while(pos < len)
	{
		while(pos < len && path[pos] != '/') pos++;
		string dir_path = path.substr(0, pos);
		struct stat stbuf;
		if(stat(dir_path.c_str(), &stbuf) < 0)
		{
			if(errno != ENOENT)
			{
				return false;
			}
			else if(mkdir(dir_path.c_str(), 0777) < 0)
			{
				return false;
			}
		}
		else if(!S_ISDIR(stbuf.st_mode))
		{
			return false;
		}
		while(pos < len && path[pos] == '/') pos++;
	}

	return true;
}

void loopDelDir(string path)
{
	DIR * dir = opendir(path.c_str());
	if(NULL == dir)
	{
		fprintf(stderr, "open %s err: %s", path.c_str(), strerror(errno));
		rmdir(path.c_str());
		return;
	}

	struct dirent * dirfile = NULL;
	while((dirfile = readdir(dir)) != NULL)
	{
		string name = dirfile->d_name;
		if(name == "." || name == "..") continue;
		name = path + "/" + name;

		if(!(dirfile->d_type & DT_DIR)) // not dir
		{
			unlink(name.c_str());
		}
		else
		{
			loopDelDir(name);
		}
	}
	closedir(dir);
	rmdir(path.c_str());
}

bool setFileLock(int fd, int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_WRLCK;
	lk.l_pid = getpid();

	if(fcntl(fd, F_SETLKW, &lk) < 0)
	{
		return false;
	}
	return true;
}

bool tryFileLock(int fd, int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_WRLCK;
	lk.l_pid = getpid();

	if(fcntl(fd, F_SETLK, &lk) < 0)
	{
		return false;
	}
	return true;
}

bool unsetFileLock(int fd, int off)
{
	struct flock lk;
	lk.l_whence = SEEK_SET;
	lk.l_start = off;
	lk.l_len = 1;
	lk.l_type = F_UNLCK;
	lk.l_pid = 0;

	if(fcntl(fd, F_SETLK, &lk) < 0)
	{
		return false;
	}
	return true;
}

void int2string(char *buf, int val)
{
	if(val < 0)
	{
		*buf = '-';
		buf++;
		val = -val;
	}

	char *p = buf;
	do
	{
		*p++ = val % 10 + '0';
		val /= 10;
	}while(val > 0);
	*p-- = '\0';

	while(buf < p)
	{
		int tmp = *buf;
		*buf++ = *p;
		*p-- = tmp;
	}
}

bool createPath(string path)
{
	string tmp = path;
	const char *str = tmp.c_str();
	char *p = (char*)str;
	while(*++p)
	{
		if(*p == '/')
		{
			*p = '\0';
			if(access(str, F_OK) < 0 && mkdir(str, 0755) < 0)
			{
				return false;
			}
			*p = '/';
		}
	}
	if(access(str, F_OK) < 0 && mkdir(str, 0755) < 0)
	{
		return false;
	}
	return true;
}

const char* getDirString(int dir)
{
	switch (dir)
	{
		case AT_CHAR_Buy:
			return "BUY";
		case AT_CHAR_Sell:
			return "SELL";
		default:
			return "Unknown dir";
	}
}

const char* getOffString(int off)
{
	switch (off)
	{
		case AT_CHAR_Open:
			return "OPEN";
		case AT_CHAR_Close:
			return "CLOSE";
		case AT_CHAR_ForceClose:
			return "FORCE_CLOSE";
		case AT_CHAR_CloseToday:
			return "CLOSE_TD";
		case AT_CHAR_CloseYesterday:
			return "CLOSE_YD";
		case AT_CHAR_ForceOff:
			return "FORCE_OFF";
		case AT_CHAR_LocalForceClose:
			return "LOCAL_FORCE_CLOSE";
		case AT_CHAR_Non:
			return "NON";
		case AT_CHAR_Auto:
			return "AUTO";
		default:
			return "Unknown off";
	}
}

string getEmOrderRtnTypeString(int type)
{
	string ret;
	if (type == static_cast<int>(emOrderRtnType::NOT_SET))
	{
		ret = "NOT_SET;";
	}
	if (type & ODS(CLOSED))
	{
		ret += "CLOSED;";
	}
	if (type & ODS(SEND))
	{
		ret += "SEND;";
	}
	if (type & ODS(TDSEND))
	{
		ret += "TDSEND;";
	}
	if (type & ODS(CXLING))
	{
		ret += "CXLING;";
	}
	if (type & ODS(ACCEPT))
	{
		ret += "ACCEPT;";
	}
	if ((type & ODS(REJECT)) == ODS(REJECT))
	{
		ret += "REJECT;";
	}
	if (type & ODS(MARKET_ACCEPT))
	{
		ret += "MARKET_ACCEPT;";
	}
	if ((type & ODS(MARKET_REJECT)) == ODS(MARKET_REJECT))
	{
		ret += "MARKET_REJECT;";
	}
	if (type & ODS(EXECUTION))
	{
		ret += "EXECUTION;";
	}
	if (type & ODS(CANCEL_REJECT))
	{
		ret += "CANCEL_REJECT;";
	}
	if ((type & ODS(CANCELED)) == ODS(CANCELED))
	{
		ret += "CANCELED;";
	}
	if ((type & ODS(ERR)) == ODS(ERR))
	{
		ret += "ERR;";
	}
	
	return ret;
}
