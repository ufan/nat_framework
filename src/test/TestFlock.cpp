#include <iostream>
#include <string>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "utils.h"

using namespace std;

int fd  = -1;
int size = 16;
char* buf = nullptr;

void signal_handler(int signum)
{
	if(fd>0) {
    munmap(buf, size);
    close(fd);
  }
  fd = -1;
}

void setup_signal_callback()
{
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGKILL, signal_handler);
}

void Usage(const char *cmd)
{
  cout << "Usage: " << cmd << " ";
  cout << "[-r][-w] file" << endl;

	exit(-1);
}

int main(int argc, char *argv[])
{
  bool is_lock = true;
  bool read_flag = true;
  int  offset = 0;
  char c = 'a';

  int opt = 0;
  while ( (opt = getopt(argc,argv,"uws:")) != -1 )
  {
    switch (opt) {
      case 'u': {
        is_lock = false;
        break;
      }
      case 'w': {
        read_flag = false;
        break;
      }
      case 's': {
        offset = atoi(optarg);
        break;
      }
      case '?': {
        Usage(argv[0]);
      }
      default:
        break;
    }
  }

  if ( optind == argc ) {
    Usage(argv[0]);
  }

  if ( read_flag ) {
    fd = open(argv[optind], O_RDONLY, (mode_t)0666);
  }
  else {
    fd = open(argv[optind], O_RDWR | O_CREAT, (mode_t)0666);
  }
  if (fd<0) {
    cout << "Can't open file " << argv[optind] << endl;
    return -1;
  }

	if( !read_flag )
  {
    if(is_lock && !tryFileLock(fd, offset)) {
      cout << "file already has a writer." << endl;
      close(fd); fd = -1;
      return -1;
    }
    
    if(is_lock && !setFileLock(fd, offset)) {
      cout << "file already has a writer." << endl;
      close(fd); fd = -1;
      return -1;
    }
  }
  else {
    struct flock lk;
    lk.l_whence = SEEK_SET;
    lk.l_start = offset;
    lk.l_len = 1;
    lk.l_type = F_RDLCK;
    lk.l_pid = getpid();

    if(fcntl(fd, F_SETLK, &lk) < 0)
    {
      cout << "file already has a locker." << endl;
      return -1;
    }
    cout << "Request RDLCK\n";
  }

	struct stat statbuff;
	if(fstat(fd, &statbuff) < 0)
  {
    close(fd); fd = -1;
    cout << "stat file fail, err: " << strerror(errno) << endl;
    return -1;
  }

	bool isnew = false;
	if(statbuff.st_size < size)	// a new file
  {
    isnew = true;
    if(ftruncate(fd, size) < 0)
    {
      close(fd); fd = -1;
      cout << "truncate file fail, err:" << strerror(errno) << endl;
      return -1;
    }
  }

  if (read_flag)
    buf = (char*)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  else
    buf = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if ((void*)buf == MAP_FAILED)
  {
    close(fd); fd = -1;
    cout << " mapping file to buffer err:" << strerror(errno) << endl;
    return -1;
  }

  if (read_flag) {
    for ( int i =0;i<size;i++) {
      cout << "byte " << i << ": " << buf[i] << endl;
    }
  }
  else {
    for ( int i =0;i<=offset;i++) {
      buf[i] = c;
    }
    if(!is_lock){
      string str {"This is strange!"};
      strcpy(buf, str.c_str());
    }
  }

  while(1) {
    cout << "I'm alive" << endl;
    usleep(1000000); }

  return 0;
}
