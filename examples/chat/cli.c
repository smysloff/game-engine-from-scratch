
// file: examples/chat/cli.c

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST "localhost"
#define PORT "6969"

#define REQUEST "GET / HTTP/1.1"    "\r\n" \
                "Host: " HOST       "\r\n" \
                "Connection: close" "\r\n" \
                                    "\r\n"

int
tcp_connect(const char *node, const char *service, int ai_family)
{
  int sockfd, status;
  struct addrinfo hints, *res;

  if (!node)
  {
    fprintf(stderr, "'node' connot be NULL\n");
    return -1;
  }

  if (!service)
  {
    fprintf(stderr, "service connot be NULL\n");
    return -1;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = ai_family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if ((status = getaddrinfo(node, service, &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
    return -1;
  }

  for (struct addrinfo *addr = res; addr; addr = addr->ai_next)
  {
    if ((sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
      continue;

    if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) != -1)
      break;

    close(sockfd);
    sockfd = -1;
  }

  freeaddrinfo(res);
  return sockfd;
}

const char *
get_http_status(int sockfd)
{
  static char buff[128];
  ssize_t bytes;

  if (send(sockfd, REQUEST, sizeof(REQUEST), 0) <= 0)
    return NULL;

  if ((bytes = recv(sockfd, buff, sizeof(buff) - 1, 0)) == -1)
    return NULL;

  buff[bytes] = '\0';

  for (size_t i = 0; i < bytes; ++i)
    if (buff[i] == '\r' || buff[i] == '\n')
      buff[i] = '\0';

  return buff;
}


int
main(void)
{
  int sockfd;
  const char *http_status;

  if ((sockfd = tcp_connect(HOST, PORT, AF_UNSPEC)) == -1)
  {
    fprintf(stderr, "Error: cannot connect to %s:%s\n", HOST, PORT);
    return EXIT_FAILURE;
  }

  if (!(http_status = get_http_status(sockfd)))
  {
    fprintf(stderr, "Error: cannot get http status from %s:%s\n", HOST, PORT);
    close(sockfd);
    return EXIT_FAILURE;
  }

  printf("%s\n", http_status);

  close(sockfd);
  return EXIT_SUCCESS;
}
