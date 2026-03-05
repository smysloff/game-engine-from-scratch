
// file: examples/system/sysinfo.c

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

xcb_connection_t *create_connection(bool use_xkb_ext);
void close_connection(xcb_connection_t *con);
keyboard_layout_t *get_keyboard_layout(xcb_connection_t *con);
void free_keyboard_layout(keyboard_layout_t *kbd);
const char *get_keyboard_layout_fullname(xcb_connection_t *con, keyboard_layout_t *kbd);
const char *get_keyboard_layout_shortname(xcb_connection_t *con, keyboard_layout_t *kbd);
bool use_xkb_extension(xcb_connection_t *con);
void cleanup(xcb_connection_t *con, keyboard_layout_t *kbd, char *info);

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

  con = NULL;
  kbd = NULL;
  info = NULL;

  con = create_connection(true);
  if (!con)
  {
    perror("Cannot create connection to X-Server");
    cleanup(con, kbd, info);
    return EXIT_FAILURE;
  }

  kbd = get_keyboard_layout(con);
  if (!kbd)
  {
    perror("Cannot get keyboard layout from X-Server");
    cleanup(con, kbd, info);
    return EXIT_FAILURE;
  }

  lang = get_keyboard_layout_shortname(con, kbd);
  date = get_datetime();

  info = smprintf("%s %s", lang, date);
  if (!info)
  {
    perror("Cannot create system info string");
    cleanup(con, kbd, info);
    return EXIT_FAILURE;
  }

  //  gls_set_root_window_name(&ctx, sysinfo);
  puts(info);

  cleanup(con, kbd, info);

  return EXIT_SUCCESS;
}


char *
smprintf(const char *fmt, ...)
{
  char *result;
  va_list args;
  int len;

  if (!fmt)
    return NULL;

  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if (!(result = malloc(++len)))
    return NULL;

  va_start(args, fmt);
  vsnprintf(result, len, fmt, args);
  va_end(args);

  return result;
}


xcb_connection_t *
create_connection(bool use_kbd_ext)
{
  xcb_connection_t *con;

  con = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(con))
    return NULL;

  if (use_kbd_ext)
    if (!use_xkb_extension(con))
      return NULL;

  return con;
}

void
close_connection(xcb_connection_t *con)
{
  if (con)
    xcb_disconnect(con);
}

keyboard_layout_t *
get_keyboard_layout(xcb_connection_t *con)
{
  keyboard_layout_t *kbd;

  if (con)
    return NULL;

  kbd = malloc(sizeof(*kbd));
  if (!kbd)
  {
    perror("Cannot allocate memory for keyboard_layout_t");
    return NULL;
  }

  kbd->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!kbd->context)
  {
    perror("Cannot create new XKB context");
    return NULL;
  }

  kbd->device_id = xkb_x11_get_core_keyboard_device_id(con);
  if (kbd->device_id == -1)
  {
    perror("Cannot get keyboard device id");
    return NULL;
  }

  kbd->keymap = xkb_x11_keymap_new_from_device(
    kbd->context, con, kbd->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!kbd->keymap)
  {
    perror("Cannot get keyboard keymap");
    return NULL;
  }

  kbd->state = xkb_x11_state_new_from_device(kbd->keymap, con, kbd->device_id);
  if (!kbd->state)
  {
    perror("Cannot get XKB state");
    return NULL;
  }

  return kbd;
}

void
free_keyboard_layout(keyboard_layout_t *kbd)
{
  if (kbd)
  {
    if (kbd->state)
      xkb_state_unref(kbd->state);

    if (kbd->keymap)
      xkb_keymap_unref(kbd->keymap);

    if (kbd->context)
      xkb_context_unref(kbd->context);

    free(kbd);
  }
}

const char *
get_keyboard_layout_fullname(xcb_connection_t *con, keyboard_layout_t *kbd)
{
  const char *buffer;
  xcb_xkb_get_state_cookie_t cookie;
  xcb_xkb_get_state_reply_t *reply;

  if (!con || !kbd)
    return NULL;

  cookie = xcb_xkb_get_state(con, kbd->device_id);
  reply = xcb_xkb_get_state_reply(con, cookie, NULL);
  if (!reply)
  {
    perror("Cannot get state reply");
    return NULL;
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

  buffer[0] = '?';
  buffer[1] = '?';
  buffer[2] = '\0';

  if (!con || !kbd)
    return buffer;

  fullname = get_keyboard_layout_fullname(con, kbd);

  if (fullname && strlen(fullname) >= 2)
  {
    buffer[0] = tolower(fullname[0]);
    buffer[1] = tolower(fullname[1]);
  }

  return buffer;
}

bool
use_xkb_extension(xcb_connection_t *con)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  if (!con)
    return false;

  cookie = xcb_xkb_use_extension(con,
    XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  reply = xcb_xkb_use_extension_reply(con, cookie, NULL);

  if (!reply || !reply->supported)
    return false;

  free(reply);

  return true;
}

void
cleanup(xcb_connection_t *con, keyboard_layout_t *kbd, char *info)
{
  if (info)
    free(info);

  if (kbd)
    free_keyboard_layout(kbd);

  if (con)
    close_connection(con);
}


const char *
get_hostname(void)
{
  static char buffer[HOST_NAME_MAX + 1];
  return (gethostname(buffer, HOST_NAME_MAX) ? NULL : buffer);
}

const char *
get_login(void)
{
  struct passwd *pw;
  pw = getpwuid(getuid());
  return ((pw && pw->pw_name) ? pw->pw_name : NULL);
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
