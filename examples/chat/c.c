// test_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int
main(void)
{
  int sock;
  struct sockaddr_in addr;
  char buff[32];
  ssize_t bytes;

  sock = socket(PF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(3000);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  connect(sock, (struct sockaddr *)&addr, sizeof(addr));

  strncpy(buff, "Hello, World!", sizeof(buff));
  send(sock, buff, strlen(buff), 0);
  bytes = recv(sock, buff, sizeof(buff), 0);
  buff[bytes] = '\0';

  printf("%s", buff);

  close(sock);
  return EXIT_SUCCESS;
}
