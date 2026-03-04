
// file: examples/system/sysinfo.c

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>


typedef struct keyboard_layout_s
{
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  int32_t device_id;
} keyboard_layout_t;

char *smprintf(const char *fmt, ...);

xcb_connection_t *connect(bool use_xkb_ext);

void disconnect(xcb_connection_t *con)
  __attribute__((nonnull));

keyboard_layout_t *get_keyboard_layout(xcb_connection_t *con)
  __attribute__((nonnull));

void free_keyboard_layout(keyboard_layout_t *kbd)
  __attribute__((nonnull));

const char *get_keyboard_layout_fullname(xcb_connection_t *con, keyboard_layout_t *kbd)
  __attribute__((nonnull));

const char *get_keyboard_layout_shortname(xcb_connection_t *con, keyboard_layout_t *kbd)
  __attribute__((nonnull));

void use_xkb_extension(xcb_connection_t *con)
  __attribute__((nonnull));

const char *get_login(void);
const char *get_hostname(void);
const char *get_datetime(void);


int
main(void)
{
  char *info;
  const char *lang;
  const char *date;

  xcb_connection_t *con;
  keyboard_layout_t *kbd;

  con = connect(true);
  kbd = get_keyboard_layout(con);

  lang = get_keyboard_layout_shortname(con, kbd);
  date = get_datetime();

  if (!(info = smprintf("%s %s", lang, date)))
  {
    fprintf(stderr, "Cannot create system info string\n");
    exit(EXIT_FAILURE);
  }

  //  gls_set_root_window_name(&ctx, sysinfo);
  puts(info);

  free(info);
  free_keyboard_layout(kbd);
  disconnect(con);

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
connect(bool use_xkb_ext)
{
  xcb_connection_t *con;

  con = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(con))
  {
    perror("error!");
    fprintf(stderr, "Cannot create connection to X-Server\n");
    exit(EXIT_FAILURE);
  }
  printf("success");

  if (use_xkb_ext)
    use_xkb_extension(con);

  return con;
}

void
disconnect(xcb_connection_t *con)
{
  assert(con);
  xcb_disconnect(con);
}

keyboard_layout_t *
get_keyboard_layout(xcb_connection_t *con)
{
  keyboard_layout_t *kbd;

  assert(con);

  if (!(kbd = malloc(sizeof(*kbd))))
  {
    fprintf(stderr, "Cannot allocate memory for keyboard_layout_t\n");
    exit(EXIT_FAILURE);
  }

  if (!(kbd->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
  {
    fprintf(stderr, "Cannot create new XKB context\n");
    exit(EXIT_FAILURE);
  }

  if ((kbd->device_id = xkb_x11_get_core_keyboard_device_id(con)) == -1)
  {
    fprintf(stderr, "Cannot get keyboard device id\n");
    exit(EXIT_FAILURE);
  }

  kbd->keymap = xkb_x11_keymap_new_from_device(
    kbd->context, con, kbd->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!kbd->keymap)
  {
    fprintf(stderr, "Cannot get XKB layout\n");
    exit(EXIT_FAILURE);
  }

  kbd->state = xkb_x11_state_new_from_device(kbd->keymap, con, kbd->device_id);

  if (!kbd->state)
  {
    fprintf(stderr, "Cannot get XKB state\n");
    exit(EXIT_FAILURE);
  }

  return kbd;
}

void
free_keyboard_layout(keyboard_layout_t *kbd)
{
  assert(kbd);

  if (kbd->state)
    xkb_state_unref(kbd->state);

  if (kbd->keymap)
    xkb_keymap_unref(kbd->keymap);

  if (kbd->context)
    xkb_context_unref(kbd->context);

  free(kbd);
}

const char *
get_keyboard_layout_fullname(xcb_connection_t *con, keyboard_layout_t *kbd)
{
  const char *buffer;
  xcb_xkb_get_state_cookie_t cookie;
  xcb_xkb_get_state_reply_t *reply;

  assert(con);
  assert(kbd);

  cookie = xcb_xkb_get_state(con, kbd->device_id);
  reply = xcb_xkb_get_state_reply(con, cookie, NULL);
  if (!reply)
  {
    fprintf(stderr, "Cannot get state reply\n");
    exit(EXIT_FAILURE);
  }

  buffer = xkb_keymap_layout_get_name(kbd->keymap, reply->lockedGroup);
  free(reply);

  return buffer;
}

const char *
get_keyboard_layout_shortname(xcb_connection_t *con, keyboard_layout_t *kbd)
{
  static char buffer[3];
  const char *fullname;

  assert(con);
  assert(kbd);

  fullname = get_keyboard_layout_fullname(con, kbd);

  if (fullname && strlen(fullname) >= 2)
  {
    buffer[0] = tolower(fullname[0]);
    buffer[1] = tolower(fullname[1]);
  }
  else
  {
    buffer[0] = '?';
    buffer[1] = '?';
  }
  buffer[2] = '\0';

  return buffer;
}

void
use_xkb_extension(xcb_connection_t *con)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  assert(con);

  cookie = xcb_xkb_use_extension(
    con, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  reply = xcb_xkb_use_extension_reply(con, cookie, NULL);

  if (!reply || !reply->supported)
  {
    fprintf(stderr, "Cannot use XKB extension\n");
    exit(EXIT_SUCCESS);
  }

  free(reply);
}


const char *
get_hostname(void)
{
  static char buffer[HOST_NAME_MAX + 1];
  int error;

  if ((error = gethostname(buffer, HOST_NAME_MAX)))
  {
    fprintf(stderr, "Cannot get hostname\n");
    exit(EXIT_FAILURE);
  }

  return buffer;
}

const char *
get_login(void)
{
  struct passwd *pw;

  pw = getpwuid(getuid());
  if (!pw || !pw->pw_name)
  {
    fprintf(stderr, "Cannot get login\n");
    exit(EXIT_FAILURE);
  }

  return pw->pw_name;
}

const char *
get_datetime(void)
{
  static char datetime[128];
  struct tm *timeinfo;
  time_t rawtime;
  size_t written_bytes;

  rawtime = time(NULL);
  timeinfo = localtime(&rawtime);

  written_bytes = strftime(datetime, sizeof(datetime), "%c", timeinfo);

  written_bytes
    ? (datetime[sizeof(datetime) - 1] = '\0')
    : (datetime[0] = '\0');

  return datetime;
}
