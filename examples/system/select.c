
// file: examples/system/select.c

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/time.h>
#include <sys/select.h>

#define CMD_BUFFER_SIZE 16
#define MAX_SECONDS 9

#define READ_ERR -1
#define READ_END 0

bool loop;
int seconds;
char cmd[CMD_BUFFER_SIZE];
struct timeval tv;
fd_set read_fds;
ssize_t bytes_read;

void
sanitize(char *s, ssize_t bytes_read)
{
  if (s[bytes_read - 1] == '\n')
    s[bytes_read - 1] = '\0';
  else
    s[bytes_read] = '\0';
}

ssize_t
read_cmd(void)
{
  fflush(stdout);
  bytes_read = read(0, cmd, sizeof(cmd) - 1);
  return bytes_read;
}

void
process_cmd(void)
{
  sanitize(cmd, bytes_read);
  printf("Command: %s\n", cmd);

  if (strcmp(cmd, "exit"))
  {
    seconds = 0;
    printf("Type command (or 'exit' to quit):\n");
  }

  else
  {
    loop = 0;
    printf("Exiting by command\n");
  }
}


int
main(void)
{
  printf("Type command (or 'exit' to quit):\n");
  fflush(stdout);

  seconds = 0;
  loop = true;

  while (loop)
  {
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    switch (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &tv))
    {
      case -1:
      {
        if (errno == EINTR)
          continue;
        perror("select()");
        loop = 0;
        break;
      }

      case 0:
      {
        printf("seconds: %d\n", ++seconds);
        fflush(stdout);
        if (seconds >= MAX_SECONDS)
        {
          printf("\nTimeout reached (%d seconds). Exiting.\n", MAX_SECONDS);
          loop = 0;
        }
        break;
      }

      default: // if stdin ready to read
      {
        switch (read_cmd())
        {
          case READ_ERR:
            loop = 0;
            perror("read()");
            break;

          case READ_END:
            loop = 0;
            printf("\nEOF detected. Exiting.\n"); // Ctrl+D
            break;

          default:
            process_cmd();
        }
      }

    }

  }

  printf("Exit Success\n");
  return 0;
}
