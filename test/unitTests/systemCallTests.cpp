#include "catch.hpp"
#include <sched.h>

#include <sys/utsname.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/ptrace.h>
#include <sys/reg.h>     /* For constants ORIG_EAX, etc */
#include <string.h>
#include <sys/wait.h>
#include <sys/syscall.h>    /* For SYS_write, etc */

#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>

#include <iostream>
#include <tuple>
#include <unistd.h>
#include <sys/types.h>
#include <experimental/optional>
#include <climits>

using namespace std;
using namespace experimental;

/*
 * Do not run these tests directly from the executable! Use runTests.py to run them!
 * Warning, please do not add tests "in the middle", only at the end, as the number
 * of ran tests determinies the value of our logical clock. Sorry about that.
 */

TEST_CASE("time system call", "time"){
  time_t tloc;
  syscall(SYS_time, &tloc);
  REQUIRE(7 == tloc);
}

TEST_CASE("statfs system call", "statfs"){
  struct statfs info;
  statfs("./", &info);

  REQUIRE(info.f_type == 0xEF53);
  REQUIRE(info.f_bsize == 100);
  REQUIRE(info.f_blocks == 1000);
  REQUIRE(info.f_bfree == 10000);
  REQUIRE(info.f_bavail == 5000);
  REQUIRE(info.f_files == 1000);
  REQUIRE(info.f_ffree == 1000);
  REQUIRE(info.f_fsid.__val[0] == 0);
  REQUIRE(info.f_fsid.__val[1] == 0);
  REQUIRE(info.f_namelen == 200);
  REQUIRE(info.f_frsize == 20);
  REQUIRE(info.f_flags == 1);
}

TEST_CASE("fstatfs system call", "statfs"){
  struct statfs info;
  int fd = open("./", O_RDONLY);
  fstatfs(fd, &info);

  REQUIRE(info.f_type == 0xEF53);
  REQUIRE(info.f_bsize == 100);
  REQUIRE(info.f_blocks == 1000);
  REQUIRE(info.f_bfree == 10000);
  REQUIRE(info.f_bavail == 5000);
  REQUIRE(info.f_files == 1000);
  REQUIRE(info.f_ffree == 1000);
  REQUIRE(info.f_fsid.__val[0] == 0);
  REQUIRE(info.f_fsid.__val[1] == 0);
  REQUIRE(info.f_namelen == 200);
  REQUIRE(info.f_frsize == 20);
  REQUIRE(info.f_flags == 1);
}

TEST_CASE("getpid simple case", "getpid"){
  pid_t p = getpid();
  REQUIRE(p == (pid_t) 1);
}

// Verify that starting program breakpoint is always deterministic.
TEST_CASE("starting program sbrk", "sbrk"){
  void* breakLocation = sbrk(0);
  // Hard to test, as this program grows, breakpoint moves.
  // REQUIRE(breakLocation == (void*)0x0000555555818000);
}

// FORK
// Catch does not support forking X(
// Instead we test fork through our ../samplePrograms

TEST_CASE("getrusage", "getrusage"){
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  REQUIRE(usage.ru_maxrss == LONG_MAX);
  REQUIRE(usage.ru_nsignals == LONG_MAX);
  REQUIRE(usage.ru_utime.tv_sec == 8);
}

TEST_CASE("getuid", "getuid"){
  uid_t uid = getuid();
  // Nobody
  REQUIRE(uid == 65534);
}

void statFamilyTests(struct stat statbuf){
  REQUIRE(statbuf.st_uid == 65534);
  REQUIRE(statbuf.st_dev == 1);
  REQUIRE(statbuf.st_blocks == 1);
  REQUIRE(statbuf.st_gid == 1);
}

TEST_CASE("stat", "stat"){
  struct stat statbuf;
  int ret = stat("./", &statbuf);
  statFamilyTests(statbuf);
  REQUIRE(statbuf.st_atim.tv_nsec == statbuf.st_atim.tv_sec);
  REQUIRE(statbuf.st_atim.tv_nsec == 9);
  REQUIRE(statbuf.st_atim.tv_sec == 9);
}


TEST_CASE("fstat", "fstat"){
  struct stat statbuf;
  int fd = open("./", O_RDONLY);
  int ret = fstat(fd, &statbuf);
  statFamilyTests(statbuf);
  REQUIRE(statbuf.st_atim.tv_nsec == statbuf.st_atim.tv_sec);
  REQUIRE(statbuf.st_atim.tv_nsec == 10);
  REQUIRE(statbuf.st_atim.tv_sec == 10);
}


TEST_CASE("lstat", "lstat"){
  struct stat statbuf;
  int ret = lstat("./", &statbuf);
  statFamilyTests(statbuf);
  REQUIRE(statbuf.st_atim.tv_nsec == statbuf.st_atim.tv_sec);
  REQUIRE(statbuf.st_atim.tv_nsec == 11);
  REQUIRE(statbuf.st_atim.tv_sec == 11);
}

 TEST_CASE("open", "/dev/urandom"){
   // TODO.
 }

TEST_CASE("prlimit64", "prlimit64"){
  // joe: can't compile a prlimit test on acggrid, I get:
  // "error: 'SYS_prlimit' was not declared in this scope"

  // list of all resources per https://linux.die.net/man/2/prlimit
  const int RESOURCE[] = {
    RLIMIT_AS, RLIMIT_CORE, RLIMIT_CPU, RLIMIT_DATA, 
    RLIMIT_FSIZE, RLIMIT_LOCKS, RLIMIT_MEMLOCK, RLIMIT_MSGQUEUE, 
    RLIMIT_NICE, RLIMIT_NOFILE, RLIMIT_NPROC, RLIMIT_RSS, 
    RLIMIT_RTPRIO, RLIMIT_RTTIME, RLIMIT_SIGPENDING, RLIMIT_STACK
  };
  struct rlimit limits;
  for (unsigned i = 0; i < sizeof(RESOURCE)/sizeof(RESOURCE[0]); i++) {
    syscall(SYS_prlimit64, 0, RESOURCE[i], nullptr, &limits);
    INFO("resource=" << RESOURCE[i] << " i=" << i << " &limits=" << &limits);
    REQUIRE(RLIM_INFINITY == limits.rlim_cur);
    REQUIRE(RLIM_INFINITY == limits.rlim_max);
  }
}

TEST_CASE("sysinfo", "sysinfo"){
  struct sysinfo info;
  sysinfo(&info);

  REQUIRE(info.uptime == LONG_MAX);
  REQUIRE(info.totalram == LONG_MAX);
  REQUIRE(info.freehigh == LONG_MAX);
  REQUIRE(info.loads[2] == LONG_MAX);
  REQUIRE(info.sharedram == LONG_MAX);
  REQUIRE(info.totalswap == LONG_MAX);
  REQUIRE(info.procs == SHRT_MAX);
  REQUIRE(info.freeswap == LONG_MAX);
}

TEST_CASE("utimensat", "utimensat"){
  // Huh, it's actually impossible to check if the timestamp is working
  // as a unit test since we cannot stat the timestamps!
  // int fd = open("./test.txt", O_RDWR);
  // futimens(fd, nullptr);
}

TEST_CASE("uname", "uname"){
  struct utsname buf;
  int ret = uname(&buf);
  REQUIRE(ret == 0);
  REQUIRE(strcmp(buf.sysname, "Linux") == 0);
  REQUIRE(strcmp(buf.nodename,"") == 0);
  REQUIRE(strcmp(buf.release, "4.0") == 0);
  REQUIRE(strcmp(buf.version, "#1") == 0);
  REQUIRE(strcmp(buf.machine, "x86_64") == 0);

#ifdef _GNU_SOURCE
  REQUIRE(strcmp(buf.domainname, "") == 0);
#endif
}

TEST_CASE("uid/gid", "uid/gid"){
  // Nobody.
  REQUIRE(getegid() == 65534);
  REQUIRE(getgid() == 65534);
  REQUIRE(geteuid() == 65534);
  REQUIRE(getpgrp() == 0);
}
