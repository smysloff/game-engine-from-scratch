
// file: examples/system/sysinfo.c

#include <assert.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <xcb/xkb.h>


typedef struct keyboard_layout_s
{
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  int32_t device_id;
} keyboard_layout_t;


char *smprintf(const char *fmt, ...);

xcb_connection_t *connect(void);
void disconnect(xcb_connection_t *con);
void use_xkb_extension(xcb_connection_t *con);
keyboard_layout_t *get_keyboard_layout(xcb_connection_t *con);


int
main(void)
{
  //char *sysinfo;

  xcb_connection_t *con;
  keyboard_layout_t *kbd;

  //int layout;
  //const char *login;
  //const char *hostname;
  //const char *datetime;


  con = connect();
  use_xkb_extension(con);
  kbd = get_keyboard_layout(con);


  //gls_init(&ctx);

  //login    = get_login();
  //hostname = get_hostname();

  //while (true)
  //{
  //  datetime = get_datetime(0, NULL);
  //  layout   = get_keyboard_layout(&ctx);

  //  sysinfo = smprintf("%s@%s %d %s", login, hostname, layout, datetime);
  //  gls_set_root_window_name(&ctx, sysinfo);
  //  print_string_endl(sysinfo);

  //  free(sysinfo);
  //  sleep(1);
  //}

  //disconnect(&con);

  return EXIT_SUCCESS;
}


char *
smprintf(const char *fmt, ...)
{
  char *result;
  va_list args;
  int len;

  assert(fmt);

  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  result = malloc(++len);
  assert(result);

  va_start(args, fmt);
  vsnprintf(result, len, fmt, args);
  va_end(args);

  return result;
}

xcb_connection_t *
connect(void)
{
  xcb_connection_t *con = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(con))
  {
    fprintf(stderr, "Cannot create connection to X-Server\n");
    exit(EXIT_FAILURE);
  }
  return con;
}

void
disconnect(xcb_connection_t *con)
{
  if (con)
    xcb_disconnect(con);
}

void
use_xkb_extension(xcb_connection_t *con)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  if (!con)
  {
    fprintf(stderr, "To use XKB extension you must connect to X-Server before\n");
    exit(EXIT_SUCCESS);
  }

  cookie = xcb_xkb_use_extension(con, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  reply = xcb_xkb_use_extension_reply(con, cookie, NULL);

  if (!reply || !reply->supported)
  {
    fprintf(stderr, "Cannot use XKB extension\n");
    exit(EXIT_SUCCESS);
  }

  free(reply);
}

keyboard_layout_t *
get_keyboard_layout(xcb_connection_t *con)
{
  keyboard_layout_t *kbd;

  if (!con)
  {
    fprintf(stderr, "To use XKB extension you must connect to X-Server before\n");
    exit(EXIT_FAILURE);
  }

  if ((kbd = malloc(sizeof(*kbd))))
  {
    fprintf(stderr, "Cannot allocate memory for keyboard_layout_t *");
    exit(EXIT_FAILURE);
  }

  kbd->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);


  return kbd;
}

const char *
get_login(void)
{
  struct passwd *pw;
  pw = getpwuid(getuid());
  assert(pw && pw->pw_name);
  return pw->pw_name;
}

const char *
get_hostname(void)
{
  static char buffer[HOST_NAME_MAX + 1];
  int error;

  error = gethostname(buffer, HOST_NAME_MAX);
  assert(!error);

  return buffer;
}
