#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

int withError(int returnCode, char* call);

int main(){
  int fd1 = withError(syscall(SYS_open, "temp1.txt", O_CREAT|O_WRONLY|O_TRUNC),
                     "open");

  int fd2 = withError(syscall(SYS_open, "temp2.txt", O_CREAT|O_WRONLY|O_TRUNC),
                     "open");

  struct stat stat1;
  withError(fstat(fd1, &stat1), "fstat");
  struct stat stat2;
  withError(fstat(fd2, &stat2), "fstat");

  system("rm -f temp1.txt");
  system("rm -f temp2.txt");

  printf("mtime1 tv_sec = %ld\n  tv_nsec = %ld\n",
         stat1.st_mtim.tv_sec, stat1.st_mtim.tv_nsec);
  printf("mtime2 tv_sec = %ld\n  tv_nsec = %ld\n",
         stat2.st_mtim.tv_sec, stat2.st_mtim.tv_nsec);

  return 0;
}

int withError(int returnCode, char* call){
  if(returnCode == -1){
    printf("Unable to %s file: %s", call, strerror(errno));
    exit(1);
  }

  return returnCode;
}
