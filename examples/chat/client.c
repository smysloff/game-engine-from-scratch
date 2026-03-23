
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HOST "ddevhut.com"
#define PORT "80"

#define REQUEST "GET / HTTP/1.1"    "\r\n" \
                "Host: " HOST       "\r\n" \
                "Accept: text/html" "\r\n" \
                "Connection: close" "\r\n" \
                                    "\r\n"

int
main(void)
{
  int sockfd;
  int status;
  ssize_t bytes;
  char buff[4096];
  struct addrinfo *res, *cur;
  struct addrinfo hints;
  struct sockaddr_in sockaddr;

  // set hints for getaddrinfo
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // AF_UNSCPEC -> IPv4 & IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // get addrinfo about HOST:PORT
  if ((status = getaddrinfo(HOST, PORT, &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  // get last addrinfo
  for (cur = res; cur->ai_next; cur = cur->ai_next)
    ;

  // get addr and port
  //port = ntohs( ((struct sockaddr_in *) cur->ai_addr)->sin_port );
  //addr = inet_ntoa( ((struct sockaddr_in *) cur->ai_addr)->sin_addr );

  // create socket
  if ((sockfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1)
  {
    perror("socket()");
    freeaddrinfo(res);
    return EXIT_FAILURE;
  }

  // connect to host
  if (connect(sockfd, cur->ai_addr, cur->ai_addrlen) == -1)
  {
    perror("connect()");
    freeaddrinfo(res);
    close(sockfd);
    return EXIT_FAILURE;
  }

  // send HTTP-request
  if ((bytes = send(sockfd, REQUEST, sizeof(REQUEST), 0)) <= 0)
  {
    perror("send()");
    freeaddrinfo(res);
    close(sockfd);
    return EXIT_FAILURE;
  }

  // receive HTTP-response
  if ((bytes = recv(sockfd, buff, sizeof(buff) - 1, 0)) <= 0)
  {
    perror("recv()");
    freeaddrinfo(res);
    close(sockfd);
    return EXIT_FAILURE;
  }
  buff[bytes] = '\0';

  // get status and print it
  for (size_t i = 0; i < bytes; ++i)
  {
    if (buff[i] == '\r' || buff[i] == '\n')
    {
      buff[i] = '\0';
      break;
    }
  }
  printf("%s\n", buff);

  // clean up
  freeaddrinfo(res);
  close(sockfd);

  // exit
  return EXIT_SUCCESS;
}
