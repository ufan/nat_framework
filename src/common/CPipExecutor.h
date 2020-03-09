/*
 * CPipExecutor.h
 *
 *  Created on: Aug 9, 2018
 *      Author: hongxu
 */

#ifndef SRC_COMMON_CPIPEXECUTOR_H_
#define SRC_COMMON_CPIPEXECUTOR_H_

#include <unistd.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
using namespace std;

class CPipExecutor
{
public:
	CPipExecutor();
	virtual ~CPipExecutor();

	bool create(string path, const vector<string> &args);

	void runChild(int readfd, int writefd, int errfd);

	void kill();

	int getReadFd() {return infd_;}

	int getWriteFd() {return outfd_;}

	int getErrFd() {return errfd_;}

	bool isRun();

	uint64_t getHash() {return hash_;}

	static uint64_t calcHash(string path, const vector<string> &args);

protected:
	pid_t		pid_	= -1;
	uint64_t 	hash_	= 0;
	int			infd_ 	= -1;
	int 		outfd_ 	= -1;
	int			errfd_ 	= -1;

	string			exec_;
	vector<string> 	args_;
};

class CPipExecutorManager
{
	CPipExecutorManager() {}
public:
	CPipExecutor* create(string path, const vector<string> &args);
	void kill(uint64_t hash);
	static CPipExecutorManager& instance() {return instance_;}
private:
	map<uint64_t, unique_ptr<CPipExecutor>> store_;
	static CPipExecutorManager instance_;
};

#endif /* SRC_COMMON_CPIPEXECUTOR_H_ */
