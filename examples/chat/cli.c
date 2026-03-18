
// file: examples/chat/cli.c


// ============================================================================
// Include headers
// ============================================================================

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


// ============================================================================
// Config options
// ============================================================================

#define MSG_LIMIT 8
#define MSG_LENGTH 256


// ============================================================================
// Util functions
// ============================================================================

char *
get_user_input(const char *prompt, char *buffer, size_t size)
{
  int ch;
  size_t len;

  if (!buffer || !size)
    return NULL;

  if (prompt)
    printf("%s", prompt);

  if (!fgets(buffer, size, stdin))
    return NULL;

  len = strlen(buffer);

  if (len && buffer[len - 1] != '\n')
    while ((ch = getchar()) != '\n' && ch != EOF);

  else
    while (len && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
      buffer[--len] = '\0';

  return buffer;
}


// ============================================================================
// ANSI escape codes
// ============================================================================

#define ERASE_TO_START "0"
#define ERASE_TO_END "1"
#define ERASE_ALL "2"

#define CSI "\033["

#define CUU "A" // Cursor Up
#define CUD "B" // Cursor Down
#define CUF "C" // Cursor Forward
#define CUB "D" // Cursor Back
#define CNL "E" // Cursor Next Line
#define CPL "F" // Cursor Previous Line
#define CHA "G" // Cursor Horizntal Absolute
#define CUP "H" // Cursor Position
#define ED  "J" // Erase Dislay
#define EL  "K" // Erase in Line
#define SU  "S" // Scroll Up
#define SD  "T" // Scroll Down

#define SCP "s" // Save Current Cursor Position
#define RCP "u" // Restore Saved Cursor Position


// ============================================================================
// Terminal control functions
// ============================================================================

void
set_cursor_position(int row, int col)
{
  printf(CSI "%d;%d" CUP, row, col);
}

void
erase_display(void)
{
  printf(CSI ERASE_ALL ED);
}

void
erase_current_line(void)
{
  printf(CSI ERASE_ALL EL);
}

void
erase_lines(int start, int count)
{
  if (start < 1 || count < 1)
    return;

  int end = start + count;

  printf(CSI SCP);
  for (int i = start; i < end; ++i)
  {
    set_cursor_position(i, 1);
    erase_current_line();
  }
  printf(CSI RCP);
}


// ============================================================================
// Structs: msg_list, msg_item
// ============================================================================

typedef struct msg_item_s
{
  char *name;
  char *text;
  struct msg_item_s *next;
} msg_item_t;

typedef struct msg_list_s
{
  msg_item_t *head;
  msg_item_t *tail;
} msg_list_t;

msg_item_t *
msg_item_alloc(const char *name, const char *text)
{
  msg_item_t *msg;

  if (!name || !text || !(msg = malloc(sizeof(msg_item_t))))
    return NULL;

  if (!(msg->name = malloc(strlen(name) + 1)))
  {
    free(msg);
    return NULL;
  }
  strcpy(msg->name, name);

  if (!(msg->text = malloc(strlen(text) + 1)))
  {
    free(msg->name);
    free(msg);
    return NULL;
  }
  strcpy(msg->text, text);

  msg->next = NULL;
  return msg;
}

void
msg_item_free(msg_item_t *msg)
{
  if (msg)
  {
    free(msg->name);
    free(msg->text);
    free(msg);
  }
}

void
msg_list_free(msg_list_t *list)
{
  msg_item_t *current, *next;

  if (list)
  {
    current = list->head;
    while (current)
    {
      next = current->next;
      msg_item_free(current);
      current = next;
    }
    list->head = NULL;
    list->tail = NULL;
  }
}

msg_item_t *
msg_list_append(msg_list_t *list, const char *name, const char *text)
{
  msg_item_t *msg;

  if (!list || !(msg = msg_item_alloc(name, text)))
    return NULL;

  list->tail
    ? (list->tail->next = msg) // if list not empty - set tail->next = msg
    : (list->head = msg);      // if list is  empty - set head = msg

  list->tail = msg;            // always set tail = msg
  return msg;
}

msg_item_t *
msg_list_shift(msg_list_t *list)
{
  msg_item_t *msg;

  if (!list || !list->head)
    return NULL;

  msg = list->head;
  list->head = list->head->next;

  if (!list->head)
    list->tail = NULL;

  msg->next = NULL;
  return msg;
}

void
msg_list_print(msg_list_t *list)
{
  if (list)
    for (msg_item_t *msg = list->head; msg; msg = msg->next)
      printf("%s: %s\n", msg->name, msg->text);
}


// ============================================================================
// Main function
// ============================================================================

int
main(void)
{
  char buff[MSG_LENGTH];
  char name[MSG_LENGTH];
  char host[MSG_LENGTH];
  int port;
  bool offline;
  msg_list_t messages = {0};


  erase_display();
  fflush(stdout);

  set_cursor_position(1, 1);
  get_user_input("name: ", name, sizeof(name));
  get_user_input("host: ", buff, sizeof(buff));

  char *colon;
  size_t len;

  colon = strchr(buff, ':');

  if (colon)
  {
    len = colon - buff;
    port = atoi(colon + 1);
  }
  else
  {
    len = strlen(buff);
    port = 80;
  }
  strncpy(host, buff, len);
  host[len] = '\0';

  printf("connencted to %s:%d\n", host, port);

  int sockfd;
  struct sockaddr_in addr;
  socklen_t soclken;

  offline = ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != -1);


  // @todo handle connection to server
  //       (now it just dummy logic
  //        just skip this place)
  if (offline)
  {
    printf("Cannot connect to '%s:%d'\n", host, port);
    printf("You are in offline-mode\n");
    printf("Talk to yourself? ('y' or 'n')\n");

    get_user_input("> ", buff, sizeof(buff));
    if (tolower(buff[0]) != 'y')
      return EXIT_SUCCESS;

    msg_list_append(&messages, name, "[joined the chat]");
  }

  else
  {
    //connect();
  }

  for (bool quit = false; !quit; )
  {
    // redraw msg list

    set_cursor_position(1, 1);
    erase_lines(1, MSG_LIMIT);
    set_cursor_position(1, 1);
    msg_list_print(&messages);


    // get user input

    set_cursor_position(MSG_LIMIT + 2, 1);
    erase_current_line();

    if (!get_user_input("> ", buff, sizeof(buff)))
      continue;

    if (buff[0] == '\0')
      continue;


    // handle commands

    if (!strcmp(buff, "/exit") || !strcmp(buff, "/quit"))
    {
      quit = true;
      continue;
    }


    // append new msg_item to msg_list
    if (offline)
      msg_list_append(&messages, name, buff);
  }

  msg_list_free(&messages);
  return EXIT_SUCCESS;
}
