/*
 * CShellCmd.cpp
 *
 *  Created on: 2017年9月29日
 *      Author: hongxu
 */

#include "CShellCmd.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "CWaiter.h"
#include "Logger.h"

CShellCmd::CShellCmd(CWaiter *p_owner)
    : CCommanderBase(p_owner), run_state_(START) {}

CShellCmd::~CShellCmd() {
  if (run_state_ == RUNNING) {
    kill(child_watcher_.pid, 9);
    child_watcher_.rstatus = -1;
    onChildExit();
  }
}

/**
 * @brief Process the pkg
 * @details This method is invoked by the owner waiter two times:
 *          1) First time, when this command is created, it will return
 *          2) Second time, when next pkg arrived (either new command or bye)
 *             The new msg from client is initiated by this command's child exit
 */
int CShellCmd::run(string &pkg) {
  // check if the previous cmd finished or not
  // if not, just return and wait for its finishing
  if (run_state_ == STOP) {
    return STATE_NEXT;  // previous command finished, go back and try to run
                        // the new command
  } else if (run_state_ == RUNNING) {
    return STATE_ABORT;  // something unexpected occurs
  }

  // create a pipe connecting from child stdout/stderr to parent fds[0]
  int fds[2];
  if (pipe(fds) < 0) {
    LOG_ERR("create pipe err: %s", strerror(errno));
    return STATE_ABORT;
  }

  // fork a child process which will be replaced by exec after setting the pipe
  pid_t pid = fork();
  if (pid < 0) {
    close(fds[0]);
    close(fds[1]);
    LOG_ERR("fork err: %s", strerror(errno));
    return STATE_ABORT;
  }

  if (pid > 0) {
    child_watcher_.data = this;
    ev_child_init(&child_watcher_, child_cb, pid, 0);
    ev_child_start(EV_DEFAULT_ & child_watcher_);  // watch child's finish

    close(fds[1]);
    read_watcher_.data = this;
    ev_io_init(&read_watcher_, read_cb, fds[0],
               EV_READ);  // watch child's stdout and stderr
    ev_io_start(EV_DEFAULT_ & read_watcher_);

    run_state_ = RUNNING;  // flag running

    // ask the owner process to wait for new msg from client
    // which will be next command or bye msg
    return STATE_PENDING;
  } else  // child
  {
    // try redirect the stdout and stderr to fds[1], exit if failed
    for (int i = 0; i < 1024; i++) {
      if (i != fds[1]) close(i);
    }
    if (0 > dup2(fds[1], STDOUT_FILENO) || 0 > dup2(fds[1], STDERR_FILENO)) {
      LOG_ERR("dup2 err: %s", strerror(errno));
      exit(-1);
    }

    // detach current code path and replace with exec's command
    if (0 > execlp("/bin/sh", "sh", "-c", pkg.c_str() + sizeof(tCommand),
                   (char *)0)) {
      fprintf(stderr, "exec err: %s\n", strerror(errno));
      exit(-1);
    }
  }
  return STATE_ABORT;
}

void CShellCmd::child_cb(EV_P_ ev_child *w, int revents) {
  CShellCmd *p_obj = (CShellCmd *)w->data;
  p_obj->onChildExit();
}

// In the parent process: send finish message to client, when executed command
// exit
void CShellCmd::onChildExit() {
  ev_child_stop(EV_DEFAULT_ & child_watcher_);
  ev_io_stop(EV_DEFAULT_ & read_watcher_);
  close(read_watcher_.fd);
  run_state_ = STOP;

  // send exit status to client: failed or success
  if (WEXITSTATUS(child_watcher_.rstatus) != 0)
    p_owner_->sayFinish(TYPE_FINISH_FAIL);
  else
    p_owner_->sayFinish();
}

// In the parent process: relay the output of the executed command to client
void CShellCmd::onRead() {
  int fd = read_watcher_.fd;

#define READ_BLOCK 1024
  read_buf_.resize(READ_BLOCK + 1);  // first byte for cmd
  int ret = 0;
  while (0 < (ret = read(fd, (char *)read_buf_.data() + 1, READ_BLOCK))) {
    read_buf_.resize(ret + 1);
    read_buf_[0] = CMD_ALLDATA;
    p_owner_->desSendData(read_buf_);
    read_buf_.resize(READ_BLOCK + 1);
  }
  read_buf_.clear();
#undef READ_BLOCK
}

void CShellCmd::read_cb(EV_P_ ev_io *w, int revents) {
  CShellCmd *p_obj = (CShellCmd *)w->data;
  return p_obj->onRead();
}
