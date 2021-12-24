/*
 * CDirReciever.cpp
 *
 *  Created on: 2018年3月12日
 *      Author: hongxu
 */

#include "CDirReciever.h"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "CConfig.h"
#include "CTunnelAgent.h"
#include "CWaiter.h"
#include "utils.h"

CDirReciever::CDirReciever(CWaiter *p_owner)
    : CCommanderBase(p_owner), is_first_pkg_(true) {}

CDirReciever::~CDirReciever() {}

int CDirReciever::run(string &pkg) {
  if (is_first_pkg_) {
    is_first_pkg_ = false;
    dirname_ = pkg.substr(sizeof(tCommand));
    if (dirname_.size() <= 0)  // empty directory
    {
      return STATE_FINISH;
    }
    if (!makedir()) {
      LOG_INFO("makedir %s err", dirname_.c_str());
      return STATE_ABORT;
    }
    return STATE_PENDING;
  }

  return processPkg(pkg);  // wait for left content
}

// base_dir/.tmp/basename_timestamp
bool CDirReciever::makedir() {
  if (dirname_.back() != '/') dirname_.push_back('/');

  string var("$TMP/");
  if (dirname_.find(var) == 0)  // $TMP 为内置变量，会被替换为临时路径
  {
    CConfig *conf = CTunnelAgent::instance()->getConfig();
    string base_dir = conf->getVal<string>("COMMON", "base_dir");
    if (base_dir[base_dir.size() - 1] != '/') {
      base_dir.push_back('/');
    }
    string dir_path = base_dir + ".tmp/";
    dirname_ = dir_path + dirname_.substr(var.size());
  }

  return createDirTree(dirname_);
}

bool CDirReciever::writeFile() {
  struct stat file_st = p_file_reciever_->getFileStat();
  string name = dirname_ + p_file_reciever_->getName();

  // if it's a directory, create it
  if (S_ISDIR(file_st.st_mode)) {
    if (mkdir(name.c_str(), file_st.st_mode) < 0) {
      LOG_INFO("mkdir %s err: %s", name.c_str(), strerror(errno));
      return false;
    }
    return true;
  }

  // if it's a file, create and write the file conent
  int fd = open(name.c_str(), O_CREAT | O_TRUNC | O_CLOEXEC | O_RDWR,
                file_st.st_mode);
  if (fd < 0) {
    LOG_INFO("create file %s err: %s", name.c_str(), strerror(errno));
    return false;
  }

  string &content = p_file_reciever_->getContent();
  int ret = write(fd, content.data(), content.size());
  if (ret != content.size()) {
    LOG_INFO("write file err: %s", strerror(errno));
    return false;
  }
  return true;
}

int CDirReciever::processPkg(string &pkg) {
  if (!p_file_reciever_) {
    if (pkg.size() < sizeof(tCommand)) {
      LOG_ERR("pkg data format err");
      return STATE_ABORT;
    }
    tCommand *p_cmd = (tCommand *)pkg.data();
    if (p_cmd->cmd == CMD_FINISH) return STATE_FINISH;

    p_file_reciever_.reset(new CFileReciever(p_owner_));
  }

  int iret = p_file_reciever_->run(pkg);
  switch (iret) {
    case STATE_ABORT:
    case STATE_PENDING:
      return iret;

    case STATE_FINISH:
      if (!writeFile()) return STATE_ABORT;
      p_file_reciever_.reset();
      return STATE_PENDING;

    default:
      LOG_ERR("unknow state %d", iret);
  }

  return STATE_ABORT;
}
