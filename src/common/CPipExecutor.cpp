/*
 * CPipExecutor.cpp
 *
 *  Created on: Aug 9, 2018
 *      Author: hongxu
 */

#include "CPipExecutor.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Logger.h"
#include "MurmurHash2.h"

extern char **environ;

CPipExecutor::CPipExecutor() {}

CPipExecutor::~CPipExecutor() { kill(); }

uint64_t CPipExecutor::calcHash(string path, const vector<string> &args) {
  uint64_t hash = MurmurHash64A(path.c_str(), path.size(), 0);
  for (auto &i : args) {
    hash = MurmurHash64A(i.c_str(), i.size(), hash);
  }
  return hash;
}

/**
 * @brief Start a child process running the passed in command and args,
 * with stdin, stdout, stderr of the child redirected to the parent.
 * @param[in] path path to the command
 * @param[in] args arguments of the command
 * @return always return true in parent, never return from this function in
 * child
 */
bool CPipExecutor::create(string path, const vector<string> &args) {
  hash_ = calcHash(path, args);
  exec_ = path;
  args_ = args;

  int readpipe[2];
  int writepipe[2];
  int errpipe[2];
  if (pipe(readpipe) < 0) {
    LOG_ERR("create read pipe err:%s", strerror(errno));
    return false;
  }
  if (pipe(writepipe) < 0) {
    close(readpipe[0]);
    close(readpipe[1]);
    LOG_ERR("create write pipe err:%s", strerror(errno));
    return false;
  }
  if (pipe(errpipe) < 0) {
    close(readpipe[0]);
    close(readpipe[1]);
    close(writepipe[0]);
    close(writepipe[1]);
    LOG_ERR("create write pipe err:%s", strerror(errno));
    return false;
  }

  pid_ = fork();
  if (pid_ < 0) {
    close(readpipe[0]);
    close(readpipe[1]);
    close(writepipe[0]);
    close(writepipe[1]);
    close(errpipe[0]);
    close(errpipe[1]);
    LOG_ERR("fork err: %s", strerror(errno));
    return false;
  }

  // Close duplicated descriptors, and leaves three pipes with:
  // 1. parent outfd -> child stdin
  // 2. child  stdout -> parent infd
  // 3. child  stderr -> parent errfd
  if (pid_ > 0) {  // parent
    infd_ = readpipe[0];
    close(readpipe[1]);
    outfd_ = writepipe[1];
    close(writepipe[0]);
    errfd_ = errpipe[0];
    close(errpipe[1]);
    return true;
  } else  // child
  {
    close(readpipe[0]);
    close(writepipe[1]);
    close(errpipe[0]);
    runChild(writepipe[0], readpipe[1], errpipe[1]);
  }
  return false;
}

// Run the child process with:
// 1. replace the original child process with new process running user passed
// command and arguments.
// 2. stdin, stdout, stderr are redirected to the pipes between parent and
// child, thus the parent will receive all information of the new child process
void CPipExecutor::runChild(int readfd, int writefd, int errfd) {
  // ensure small file descriptors (stdio, stdout, stderr) available?
  for (int i = 0; i < 1024; i++) {
    if (i != readfd && i != writefd && i != errfd) {
      close(i);
    }
  }

  // replace stdin, stdout, stderr with the pipe end connected to the  parent
  dup2(readfd, STDIN_FILENO);
  dup2(writefd, STDOUT_FILENO);
  dup2(errfd, STDERR_FILENO);
  close(readfd);
  close(writefd);
  close(errfd);

  // replace the child process with a copy process running the input command
  // the stdin, stdout, stderr of the command now redirects to the parent
  const char **p_args = new const char *[args_.size() + 2];
  p_args[0] = exec_.c_str();
  for (uint32_t i = 0; i < args_.size(); ++i) p_args[i + 1] = args_[i].c_str();
  p_args[args_.size() + 1] = nullptr;

  if (0 >
      execvpe(exec_.c_str(), (char *const *)p_args, (char *const *)environ)) {
    exit(-1);
  }
}

// check if child process is running or still running
bool CPipExecutor::isRun() {
  if (pid_ > 0) {
    int status = 0;
    if (waitpid(pid_, &status, WNOHANG) == pid_) {
      pid_ = -1;
      return false;
    }
    return true;
  }
  return false;
}

// send terminate signal to child process and release resources
// if child process is not started successfully, do nothing
void CPipExecutor::kill() {
  if (pid_ > 0) {
    ::kill(pid_, SIGTERM);
    pid_ = -1;
  }
  if (infd_ >= 0) {
    close(infd_);
    infd_ = -1;
  }
  if (outfd_ >= 0) {
    close(outfd_);
    outfd_ = -1;
  }
  if (errfd_ >= 0) {
    close(errfd_);
    errfd_ = -1;
  }
}

CPipExecutorManager CPipExecutorManager::instance_;

CPipExecutor *CPipExecutorManager::create(string path,
                                          const vector<string> &args) {
  auto hash = CPipExecutor::calcHash(path, args);
  if (store_.find(hash) == store_.end()) {
    unique_ptr<CPipExecutor> p(new CPipExecutor());
    if (not p->create(path, args)) return nullptr;
    store_[hash] = move(p);
  }
  return store_[hash].get();
}

void CPipExecutorManager::kill(uint64_t hash) {
  auto iter = store_.find(hash);
  if (iter != store_.end()) {
    store_.erase(iter);
  }
}
