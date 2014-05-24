#include "ud_ucase.h"


int main(int argc, char const *argv[])
{
  struct sockaddr_un svaddr, claddr;
  int sfd;
  socklen_t len;
  ssize_t num_bytes;
  char buf[BUF_SIZE];

  sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sfd == -1)
  {
    errExit("create socket error!");
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(&svaddr.sun_path[1], SV_SOCK_NAME, sizeof(svaddr.sun_path) - 2);

  if (bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
    errExit("bind error");
  }

  for (;;) {
    num_bytes = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr*)&claddr, &len);
    if (num_bytes == -1)
    {
      errExit("recv error");
    }
    buf[num_bytes] = '\0';
    printf("recv: %zd bytes from %s\n%s\n\n", num_bytes, claddr.sun_path, buf);
    sleep(1);
  }


  return 0;
}
