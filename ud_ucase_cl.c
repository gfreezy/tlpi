#include "ud_ucase.h"


int main(int argc, char const *argv[])
{

  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    usageErr("%s msg...\n", argv[0]);
  }

  struct sockaddr_un svaddr, claddr;
  int cfd;
  ssize_t bytes_sent;

  cfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (cfd == -1)
  {
    errExit("create socket error");
  }

  memset(&claddr, 0, sizeof(struct sockaddr_un));
  claddr.sun_family = AF_UNIX;
  snprintf(claddr.sun_path, sizeof(claddr.sun_path), "/tmp/ud_ucase_cl.%ld", (long)getpid());

  if (bind(cfd, (struct sockaddr *)&claddr, sizeof(claddr)) == -1)
  {
    errExit("bind error");
  }

  memset(&svaddr, 0, sizeof(svaddr));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

  size_t slen = strlen(argv[1]);
  bytes_sent = sendto(cfd, argv[1], slen, 0, (struct sockaddr *)&svaddr, sizeof(svaddr));
  if (bytes_sent != slen)
  {
    errExit("send error");
  }

  return 0;
}
