
// file: examples/chat/srv.c


// ============================================================================
// Include headers
// ============================================================================


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


// ============================================================================
// Config options
// ============================================================================

#define MSG_LIMIT 8
#define MSG_LENGTH 256


// ============================================================================
// Main function
// ============================================================================

int
main(void)
{
  //int pool[8]; // sockets pool
  int connfd;  // connection or connector
  int clientfd;

  if ((connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("socket");
    return EXIT_FAILURE;
  }
  //pool[0] = connfd;

  struct sockaddr_in conn_addr;
  conn_addr.sin_family = AF_INET;
  conn_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  conn_addr.sin_port = htons(3000);

  if (bind(connfd, (struct sockaddr *) &conn_addr, sizeof(conn_addr)) == -1)
  {
    perror("bind");
    close(connfd);
    return EXIT_FAILURE;
  }

  if (listen(connfd, SOMAXCONN) == -1)
  {
    perror("listen");
    close(connfd);
    return EXIT_FAILURE;
  }

  for (bool quit = false; !quit; )
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    clientfd = accept(connfd, (struct sockaddr *) &client_addr, &client_len);

    char buffer[32] = {0};
    strcpy(buffer, "[joined the chat]\n");
    while (1)
    {
      send(clientfd, buffer, strlen(buffer) + 1, 0);
      sleep(2);
    }
  }

  // cleanup and exit
  close(connfd);
  return EXIT_SUCCESS;
}
