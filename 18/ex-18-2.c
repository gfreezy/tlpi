#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "tlpi_hdr.h"


int main(int argc, char const *argv[])
{
  int fd;
  if (-1 == mkdir("test", S_IRUSR|S_IWUSR|S_IXUSR)) {
    errExit("mkdir");
  }

  if (-1 == chdir("test")) {
    errExit("chdir");
  }

  if (-1 == (fd = open("myfile", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))) {
    errExit("open");
  }

  if (-1 == symlink("myfile", "../mylink")) {
    errExit("symlink");
  }

  if (-1 == chmod("../mylink", S_IRUSR)) {
    errExit("chmod");
  }
  return 0;
}
