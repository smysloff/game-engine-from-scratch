
// file: examples/chat/srv.c


// ============================================================================
// Include headers
// ============================================================================


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>


// ============================================================================
// Config options
// ============================================================================

#define HOST INADDR_ANY
#define PORT 6969
#define MSG_LEN 256
#define FDS_MAX 8


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
  bool quit;
  int opt;


  for (int i = 0; i < FDS_MAX; ++i)
    clients[i] = -1;

  socklen = sizeof(addr);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = PF_INET;
  addr.sin_addr.s_addr = htonl(HOST);
  addr.sin_port = htons(PORT);

  if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  opt = 1;
  if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
  {
    perror("setsockopt");
    close(server);
    return EXIT_FAILURE;
  }

  if (bind(server, (struct sockaddr *) &addr, sizeof(addr)) == -1)
  {
    perror("bind");
    close(server);
    return EXIT_FAILURE;
  }

  if (listen(server, SOMAXCONN) == -1)
  {
    perror("listen");
    close(server);
    return EXIT_FAILURE;
  }

  printf("Chat server started on port %d\n", PORT);


  maxfd = server;
  quit = false;

  while (!quit)
  {
    FD_ZERO(&readfds);
    FD_SET(server, &readfds);

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
