#include "readline.h"


ssize_t readline(int fd, char *buf, int buf_size) {
  char c;
  int read_size = 0;
  ssize_t n;

  while ((n = read(fd, &c, 1)) > 0) {
    read_size++;
    if (read_size < buf_size) {
      *buf = c;
      buf++;
    }
    if (c == '\n')
    {
      break;
    }
  }
  *buf = '\0';
  if (n == -1)
  {
    return -1;
  }
  return read_size;
}
