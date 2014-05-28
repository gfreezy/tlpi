#define _BSD_SOURCE

#include <netdb.h>
#include "is_seqnum.h"

#define BACKLOG 50


int main(int argc, char const *argv[])
{
  uint32_t seq_num = 0;
  char req_len_str[INT_LEN];
  char seq_num_str[INT_LEN];
  struct sockaddr_storage claddr;
  int lfd, cfd, optval, req_len;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
  char addr_str[ADDRSTRLEN];
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];

  if (argc == 2)
  {
    if (strcmp(argv[1], "-h") == 0)
    {
      usageErr("%s [seqnum]\n", argv[0]);
    }
    seq_num = getInt(argv[1], GN_ANY_BASE, "sequence number");
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
  {
    errExit("signal");
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_flags = AI_PASSIVE|AI_NUMERICSERV;
  if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0)
  {
    errExit("getaddrinfo");
  }

  optval = 1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (lfd == -1)
    {
      continue;
    }
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
      errExit("set socket");
    }
    if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
    {
      break;
    }
    close(lfd);
  }

  if (rp == NULL)
  {
    fatal("could not bind to any addresses");
  }

  freeaddrinfo(result);

  if (listen(lfd, BACKLOG) == -1)
  {
    errExit("listen");
  }

  for (;;) {
    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
    if (cfd == -1)
    {
      errMsg("accept");
      continue;
    }
    if (getnameinfo((struct sockaddr *)&claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST) == 0) {
      snprintf(addr_str, ADDRSTRLEN, "%s, %s", host, service);
    } else {
      snprintf(addr_str, ADDRSTRLEN, "(?UNKNOWN?)");
    }

    printf("connection from %s\n", addr_str);

    if (readline(cfd, req_len_str, INT_LEN) <= 0)
    {
      close(cfd);
      continue;
    }
    req_len = atoi(req_len_str);
    if (req_len <= 0)
    {
      close(cfd);
      continue;
    }

    snprintf(seq_num_str, INT_LEN, "%d\n", seq_num);
    if (write(cfd, seq_num_str, strlen(seq_num_str)) != strlen(seq_num_str))
    {
      errMsg("write");
      close(cfd);
      continue;
    }
    seq_num += req_len;
    if (close(cfd) == -1)
    {
      errExit("close");
    }
  }

  if (close(lfd) == -1)
  {
    errExit("close");
  }


  return 0;
}
