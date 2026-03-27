
// file: examples/chat/srv.c


// ============================================================================
// Include headers
// ============================================================================


//#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>


// ============================================================================
// Config options
// ============================================================================

#define HOST INADDR_ANY
#define PORT 6969
#define MSG_LEN 256
#define FDS_MAX 8


// ============================================================================
// Functions
// ============================================================================

void
set_socket_reuse(int sockfd)
{
  int opt;
  if (sockfd > STDERR_FILENO)
  {
    opt = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) == -1)
    {
      perror("setsockopt");
      close(sockfd);
      exit(EXIT_FAILURE);
    }
  }
}

int
create_socket_tcp(void)
{
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  return sockfd;
}

void
start_socket_on(int sockfd, int port)
{
  struct sockaddr_in addr;
  socklen_t socklen;

  socklen = sizeof(addr);
  set_socket_reuse(sockfd);

  addr.sin_family = PF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
  {
    perror("bind");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, SOMAXCONN) == -1)
  {
    perror("listen");
    close(sockfd);
    exit(EXIT_FAILURE);
  }
}


// ============================================================================
// Main function
// ============================================================================

int
main(void)
{
  int server;
  int client;
  int clients[FDS_MAX];
  fd_set readfds;
  int maxfd;
  struct sockaddr_in addr;
  socklen_t socklen;
  struct timeval delay;
  char buff[MSG_LEN];


  for (int i = 0; i < FDS_MAX; ++i)
    clients[i] = -1;

  server = create_socket_tcp();
  start_socket_on(server, PORT);
  printf("Chat server started on port %d\n", PORT);


  for (bool loop = true; loop; )
  {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(server, &readfds);
    maxfd = server;

    memset(&addr, '\0', sizeof(addr));
    client = accept(server, (struct sockaddr *) &addr, &socklen);

    memset(buff, '\0', sizeof(buff));
    strcpy(buff, "[joined the chat]\n");

    while (1)
    {
      send(server, buff, strlen(buff) + 1, 0);
      sleep(2);
    }
  }

  // cleanup and exit
  close(server);
  return EXIT_SUCCESS;
}
