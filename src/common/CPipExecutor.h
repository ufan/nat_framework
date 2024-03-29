/*
 * CPipExecutor.h
 *
 *  Created on: Aug 9, 2018
 *      Author: hongxu
 */

#ifndef SRC_COMMON_CPIPEXECUTOR_H_
#define SRC_COMMON_CPIPEXECUTOR_H_

#include <unistd.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class CPipExecutor {
 public:
  CPipExecutor();
  virtual ~CPipExecutor();

  bool create(string path, const vector<string> &args);

  void runChild(int readfd, int writefd, int errfd);

  void kill();

  int getReadFd() { return infd_; }

  int getWriteFd() { return outfd_; }

  int getErrFd() { return errfd_; }

  bool isRun();

  uint64_t getHash() { return hash_; }

  static uint64_t calcHash(string path, const vector<string> &args);

 protected:
  pid_t pid_ = -1;  // process id of child in the parent process, 0 in the child
  uint64_t hash_ = 0;  // hash id from command and args string
  int infd_ = -1;      // pipe descriptor for read
  int outfd_ = -1;     // pipe descriptor for write
  int errfd_ = -1;     // pipe described for error output

  string exec_;
  vector<string> args_;
};

class CPipExecutorManager {
  CPipExecutorManager() {}

 public:
  CPipExecutor *create(string path, const vector<string> &args);
  void kill(uint64_t hash);
  static CPipExecutorManager &instance() { return instance_; }

 private:
  map<uint64_t, unique_ptr<CPipExecutor>> store_;
  static CPipExecutorManager instance_;
};

#endif /* SRC_COMMON_CPIPEXECUTOR_H_ */
