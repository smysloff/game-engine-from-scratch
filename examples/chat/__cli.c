
// file: examples/chat/__cli.c

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define NODE "example.com"
#define SERV "80"

#define REQUEST              \
  "GET / HTTP/1.1"    "\r\n" \
  "Host: " NODE       "\r\n" \
  "Connection: close" "\r\n" \
                      "\r\n"

#define BUFF_SIZE 4096


typedef struct
{
  int fd;
  struct sockaddr_storage addr;
  socklen_t len;
  char node[NI_MAXHOST];
  char serv[NI_MAXSERV];
} socket_t;

[[noreturn]] void
die(const char *msg)
{
  if (msg)
    fprintf(stderr, "%s" "\n", msg);
  exit(EXIT_FAILURE);
}

bool
connect_to(socket_t *sock, const char *node, const char *serv)
{
  int err;
  struct addrinfo hints;
  struct addrinfo *addrs, *addr;

  if (!sock || !node || !serv)
    return false;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if ((getaddrinfo(node, serv, &hints, &addrs) != 0))
    return false;

  for (addr = addrs; addr; addr = addr->ai_next)
  {
    sock->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    if (sock->fd == -1)
      continue;

    if (connect(sock->fd, addr->ai_addr, addr->ai_addrlen) == 0)
      break;

    close(sock->fd);
    sock->fd = -1;
  }

  if (sock->fd != -1) // if addr == NULL then sock->fd == -1 always!
  {
    memcpy(&sock->addr, addr->ai_addr, addr->ai_addrlen);
    sock->len = addr->ai_addrlen;

    err = getnameinfo(
      (struct sockaddr *) &sock->addr, sock->len,
      sock->node, sizeof(sock->node),
      sock->serv, sizeof(sock->serv),
      NI_NUMERICHOST | NI_NUMERICSERV
    );

    if (err)
    {
      close(sock->fd);
      sock->fd = -1;
    }
  }

  freeaddrinfo(addrs);
  return sock->fd != -1;
}


int
main(void)
{
  socket_t sock;
  ssize_t bytes;
  char buff[BUFF_SIZE];


  if (!connect_to(&sock, NODE, SERV))
    die("Cannot connect to server");


  // send request

  if ((bytes = send(sock.fd, REQUEST, sizeof(REQUEST) - 1, 0)) <= 0)
  {
    perror("send()");
    close(sock.fd);
    exit(EXIT_FAILURE);
  }


  // receive response

  if ((bytes = recv(sock.fd, buff, sizeof(buff) - 1, 0)) <= 0)
  {
    perror("recv()");
    close(sock.fd);
    exit(EXIT_FAILURE);
  }


  // and extract status message from response

  buff[bytes] = '\0';

  for (size_t i = 0; i < bytes; ++i)
  {
    if (buff[i] == '\r' || buff[i] == '\n')
    {
      buff[i] = '\0';
      break;
    }
  }


  // print result

  printf("user wants: %s:%s" "\n", NODE, SERV);
  printf("connect to: %s:%s" "\n", sock.node, sock.serv);
  printf("get status: %s"    "\n", buff);


  // cleanup and exit

  close(sock.fd);
  return EXIT_SUCCESS;
}
