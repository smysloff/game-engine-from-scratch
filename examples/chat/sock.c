
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int
main(void)
{
  int server;
  int client;
  struct sockaddr_in addr;
  socklen_t socklen;
  char buff[256];
  char *data;
  ssize_t bytes;
  size_t len;
  int opt;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(3000);

  server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  opt = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bind(server, (struct sockaddr *) &addr, sizeof(addr));
  listen(server, 8);

  socklen = sizeof(addr);
  memset(&addr, '\0', socklen);
  client = accept(server, (struct sockaddr *) &addr, &socklen);

  data = malloc(1);
  data[0] = '\0';

  for (bool loop = true; loop; )
  {
    bytes = recv(client, buff, sizeof(buff) - 1, 0);
    if (bytes == -1)
      return EXIT_FAILURE;
    buff[bytes] = '\0';

    len = strlen(data) + bytes;
    data = realloc(data, len + 1);
    strcat(data, buff);
    data[len] = '\0';

    for (size_t i = 0; i < bytes; ++i)
      if (buff[i] == '\0')
        loop = false;
  }

  printf("%s", data);
  send(client, data, len, 0);

  free(data);
  close(client);
  close(server);
  return EXIT_SUCCESS;
}
