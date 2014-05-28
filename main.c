#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include "lib/tlpi_hdr.h"
#include "lib/readline.h"

#define BUF_SIZE 100


int main(int argc, char const *argv[])
{
  if (argc < 2)
  {
    usageErr("%s filename\n", argv[0]);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1)
  {
    errExit("open error");
  }

  char buf[BUF_SIZE];
  ssize_t n_read;
  int line_num = 1;
  while ((n_read = readline(fd, buf, BUF_SIZE)) > 0) {
    if (buf[n_read - 1] == '\n') {
      printf("%.4d:\t%s", line_num, buf);
    } else {
      printf("%.4d:\t%s\n", line_num, buf);
    }
    line_num++;
  }

  return 0;
}
