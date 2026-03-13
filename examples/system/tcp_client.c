
// file: examples/system/tcp_client.c

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_ORANGE "\x1b[38;5;202m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN  "\x1b[32m"
#define ANSI_COLOR_RESET  "\x1b[0m"

#define INPUT_MAXLEN 256
#define LOGIN_MAXLEN 16
#define ADDRESS_MAXLEN 22
#define HOST_MAXLEN 16

#define HTTP_REQUEST "GET / HTTP/1.1"    "\r\n" \
                     "Host: %s"          "\r\n" \
                     "User-Agent: %s"    "\r\n" \
                     "Accept: text/html" "\r\n" \
                     "Connection: close" "\r\n" \
                                         "\r\n"


void get_user_input(const char *prompt, char *buffer, size_t size);
void copy_string(char *dst, const char *src, size_t dst_size);
void get_host_and_port(const char *address, char *host, int *port);
void create_addr(struct sockaddr_in *addr, const char *host, int port);

// create_socket
// connect_to
// send_to
// recieve_from

int
main(void)
{
  int sockfd;
  struct sockaddr_in addr;
  char host[HOST_MAXLEN];       // 10.1.250.10
  int port;                     // 8080
  char address[ADDRESS_MAXLEN]; // 10.1.250.10:8080
  //char input[INPUT_MAXLEN];
  char login[LOGIN_MAXLEN];
  char request[sizeof(HTTP_REQUEST) + HOST_MAXLEN];
  char response[32768];
  ssize_t bytes_read;


  // get login
  get_user_input("login: ", login, sizeof(login));
  //copy_string(login, input, sizeof(login));

  // get address
  get_user_input("address: ", address, sizeof(address));
  //copy_string(address, input, sizeof(address));

  get_host_and_port(address, host, &port);
  create_addr(&addr, host, port);

  // create client socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    fprintf(stderr, ANSI_COLOR_RED "Error [socket]: %s\n" ANSI_COLOR_RESET, strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("Log: create socket %d\n", sockfd);

  // connect to given address
  if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
  {
    fprintf(stderr, ANSI_COLOR_RED "Error [connect]: %s\n" ANSI_COLOR_RESET, strerror(errno));
    close(sockfd);
    exit(EXIT_FAILURE);
  }
  printf("Log: we are connected to %s:%d\n", host, port);

  // create request with host
  if (snprintf(request, sizeof(request), HTTP_REQUEST, host, login) <= 0)
  {
    fprintf(stderr, ANSI_COLOR_RED "Error [snprintf]: %s\n" ANSI_COLOR_RESET, strerror(errno));
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // send HTTP-request
  send(sockfd, request, strlen(request), 0);
  printf("Log: HTTP-Request\n%s", request);

  // recieve HTTP-response
  bytes_read = recv(sockfd, response, sizeof(response) - 1, 0);
  response[bytes_read] = '\0';
  printf("Log: HTTP-Response\n%s", response);

  close(sockfd);

  return EXIT_SUCCESS;
}


void
get_user_input(const char *prompt, char *buffer, size_t size)
{
  char *endl;
  char ch;

  if (!buffer)
    return;

  if (prompt)
  {
    printf("%s", prompt);
    fflush(stdout);
  }

  if (fgets(buffer, (int) size, stdin))              // get data from user
  {
    if ((endl = strchr(buffer, '\n')))               // if user enter correct data
      *endl = '\0';                                  // trim 'end of line' symbol
    else                                             // if user enter more data than buffer size
      while ((ch = getchar()) != '\n' && ch != EOF); // drain all data from input
  }

  else                                               // if user press Ctrl+D or we get error
    buffer[0] = '\0';                                // make buffer empty
}

void
copy_string(char *dst, const char *src, size_t dst_size)
{
  size_t i;

  for (i = 0; i < dst_size - 1 && src[i] != '\0'; ++i)
    dst[i] = src[i];
  dst[i] = '\0';

  if (src[i] != '\0')
    fprintf(stderr, ANSI_COLOR_ORANGE "Warning: string truncated (was longer than %zu characters)\n" ANSI_COLOR_RESET, dst_size - 1);
}

void
get_host_and_port(const char *address, char *host, int *port)
{
  *port = 0;
  sscanf(address, "%s:%d", host, port);
  if (*port < 1)
    *port = 80;
}

void
create_addr(struct sockaddr_in *addr, const char *host, int port)
{
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(host);
  addr->sin_port = htons(port);
}
