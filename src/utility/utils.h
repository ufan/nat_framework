/*
 * utils.h
 *
 *  Created on: Sep 21, 2017
 *      Author: hongxu
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <stdint.h>
#include "ATStructure.h"
using namespace std;


#define getCycleBegin(cycles_high, cycles_low)\
	asm volatile ("CPUID\n\t"\
	"RDTSC\n\t"\
	"mov %%edx, %0\n\t"\
	"mov %%eax, %1\n\t": "=r" ((cycles_high)), "=r" ((cycles_low))::\
	"%rax", "%rbx", "%rcx", "%rdx");


#define getCycleEnd(cycles_high, cycles_low)\
	asm volatile("RDTSCP\n\t"\
	"mov %%edx, %0\n\t"\
	"mov %%eax, %1\n\t"\
	"CPUID\n\t": "=r" ((cycles_high)), "=r" ((cycles_low)):: "%rax",\
	"%rbx", "%rcx", "%rdx");


#define getu64(h,l) ((((uint64_t)(h)) << 32) | (l))


#define YELLOW 		"\033[1;33m"
#define RED			"\033[0;31m"
#define BLACK		"\033[0;30m"
#define WHITE		"\033[1;37m"
#define GREEN		"\033[0;32m"
#define NOCOLOR		"\033[0m"

#define SWARN(s)	YELLOW s NOCOLOR
#define SERR(s)		RED s NOCOLOR
#define SOK(s)		GREEN s NOCOLOR


inline bool setNonBlock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	return 0 == fcntl(fd, F_SETFL, flags);
}

inline bool setBlock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	return 0 == fcntl(fd, F_SETFL, flags);
}

void setDaemon();

void splitCmdLine(string cmd, vector<string> &res);

string joinCmdVector(vector<string> cmd, string delimiter, uint32_t start_pos=0);

void setProcTitle(string title, char **p_os_argv);

bool setProcTitle(string titile);

pid_t findProcessByCmdLine(string cmdline);

void findAllProcessByCmdLine(string cmdline, vector<pid_t> &res);

bool setaffinity(int cpuid);

bool setRealtimeSchedPolicy(int prior=1);

bool createDirTree(string path);

void loopDelDir(string path);

bool setFileLock(int fd, int off);

bool tryFileLock(int fd, int off);

bool unsetFileLock(int fd, int off);

/*
 * clock format "09:12:36"
 */
inline long getSecondsFromClockStr(const char *clock)
{return clock[0] * 36000 + clock[1] * 3600 + clock[3] * 600 + clock[4] * 60 + clock[6] * 10 + clock[7] - 1933008;}

void int2string(char *buf, int val);

bool createPath(string path);

const char* getDirString(int dir);

const char* getOffString(int off);

string getEmOrderRtnTypeString(int type);

#endif /* INCLUDE_UTILS_H_ */

