
// file: examples/chat/test.c

//#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // IPPROTO_TCP
#include <arpa/inet.h> // inet_addr

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


size_t
send_message(int sockfd, const char *msg)
{
  size_t msg_len;
  uint32_t net_len;
  ssize_t sent_bytes;
  ssize_t total_bytes;

  msg_len = strlen(msg);
  net_len = htonl(msg_len);

  sent_bytes = send(sockfd, &net_len, sizeof(net_len), 0);
  if (sent_bytes != sizeof(net_len))
  {
    perror("send length");
    return -1;
  }
  total_bytes = sent_bytes;

  sent_bytes = send(sockfd, msg, msg_len, 0);
  if (sent_bytes != (ssize_t) msg_len)
  {
    perror("send message");
    return -1;
  }
  total_bytes += sent_bytes;

  return total_bytes;
}

char *
_strcpy(char *dst, const char *src, size_t size)
{
  strncpy(dst, src, size + 1);
  dst[size] = '\0';
  return dst;
}

#define HOST "127.0.0.1"
#define PORT 3000
#define NAME "genry_call_will"

int
main(void)
{
  char host[16];
  int port;
  int sockfd;
  struct sockaddr_in addr;
  char buff[256];
  ssize_t bytes;

  _strcpy(host, HOST, strlen(HOST));
  port = PORT;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  if ((connect(sockfd, (struct sockaddr *) &addr, sizeof(addr))) == -1)
  {
    perror("connect");
    close(sockfd);
    return EXIT_FAILURE;
  }

  _strcpy(buff, NAME, strlen(NAME));

  if ((bytes = send_message(sockfd, buff)) == -1)
  {
    perror("send_message");
    close(sockfd);
    return EXIT_FAILURE;
  }

  if ((bytes = recv(sockfd, buff, sizeof(buff), 0)) == -1)
  {
    perror("recv");
    close(sockfd);
    return EXIT_FAILURE;
  }
  buff[bytes] = '\0';
  printf("%s\n", buff);

  close(sockfd);
  return EXIT_SUCCESS;
}
