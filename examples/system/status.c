
// file: examples/system/status.c

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>


typedef struct connection_s
{
  xcb_connection_t *ptr;
  xcb_screen_t *screen;
} connection_t;


typedef struct keyboard_layout_s
{
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  int32_t device_id;
  uint8_t last_group;
} keyboard_layout_t;


char *smprintf(const char *fmt, ...);
connection_t *create_connection(bool use_kbd_ext);
void close_connection(connection_t *con);
bool use_xkb_extension(connection_t *con);
keyboard_layout_t *get_keyboard_layout(connection_t *con);
void free_keyboard_layout(keyboard_layout_t *kbd);
bool select_xkb_extension(connection_t *con, keyboard_layout_t *kbd);
const char *get_keyboard_layout_fullname(connection_t *con, keyboard_layout_t *kbd);
const char *get_keyboard_layout_shortname(connection_t *con, keyboard_layout_t *kbd);
const char *get_datetime(void);
bool update_status(connection_t *con, keyboard_layout_t *kbd);
bool set_status(connection_t *con, const char *status);


int
main(void)
{
  connection_t *con;
  keyboard_layout_t *kbd;

  con = create_connection(true);
  if (!con)
  {
    perror("Cannot create connection to X-Server and/or use XKB extension");
    close_connection(con);
    return EXIT_FAILURE;
  }

  kbd = get_keyboard_layout(con);
  if (!kbd)
  {
    perror("Cannot get keyboard layout");
    free_keyboard_layout(kbd);
    close_connection(con);
    return EXIT_FAILURE;
  }

  while (true)
  {
    if (!update_status(con, kbd))
    {
      free_keyboard_layout(kbd);
      close_connection(con);
      return EXIT_FAILURE;
    }
    sleep(1);
  }

  free_keyboard_layout(kbd);
  close_connection(con);
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

  result = malloc(++len);
  if (!result)
    return NULL;

  va_start(args, fmt);
  vsnprintf(result, len, fmt, args);
  va_end(args);

  return result;
}


connection_t *
create_connection(bool use_kbd_ext)
{
  connection_t *con;

  con = malloc(sizeof(*con));
  if (!con)
    return NULL;

  con->ptr = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(con->ptr))
  {
    close_connection(con);
    return NULL;
  }

  con->screen = xcb_setup_roots_iterator(xcb_get_setup(con->ptr)).data;

  if (use_kbd_ext)
  {
    if (!use_xkb_extension(con))
    {
      close_connection(con);
      return NULL;
    }
  }

  return con;
}


void
close_connection(connection_t *con)
{
  if (con)
  {
    if (con->ptr)
      xcb_disconnect(con->ptr);
    free(con);
  }
}


bool
use_xkb_extension(connection_t *con)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  if (!con || !con->ptr)
    return false;

  cookie = xcb_xkb_use_extension(con->ptr,
    XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  reply = xcb_xkb_use_extension_reply(con->ptr, cookie, NULL);

  if (!reply || !reply->supported)
    return false;

  free(reply);
  return true;
}


keyboard_layout_t *
get_keyboard_layout(connection_t *con)
{
  keyboard_layout_t *kbd;

  if (!con || !con->ptr)
    return NULL;

  kbd = malloc(sizeof(*kbd));
  if (!kbd)
    return NULL;

  kbd->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!kbd->context)
  {
    free_keyboard_layout(kbd);
    return NULL;
  }

  kbd->device_id = xkb_x11_get_core_keyboard_device_id(con->ptr);
  if (kbd->device_id == -1)
  {
    free_keyboard_layout(kbd);
    return NULL;
  }

  kbd->keymap = xkb_x11_keymap_new_from_device(
    kbd->context, con->ptr, kbd->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!kbd->keymap)
  {
    free_keyboard_layout(kbd);
    return NULL;
  }

  kbd->state = xkb_x11_state_new_from_device(kbd->keymap, con->ptr, kbd->device_id);
  kbd->last_group = 0;

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


bool
select_xkb_extension(connection_t *con, keyboard_layout_t *kbd)
{
  xcb_void_cookie_t cookie;
  xcb_generic_error_t *error;

  uint16_t affect_which;
  uint16_t clear;
  uint16_t select_all;
  uint16_t affect_map;
  uint16_t map;
  void *details;

  if (!con || !con->ptr || !kbd)
    return false;

  // какие события хотим получать
  affect_which = XCB_XKB_EVENT_TYPE_STATE_NOTIFY;

  // какие события очищаем
  clear = 0;

  // какие события выбираем
  select_all = XCB_XKB_EVENT_TYPE_STATE_NOTIFY;

  // настройки map событий и деталей
  affect_map = 0;
  map = 0;
  details = NULL;

  cookie = xcb_xkb_select_events_checked(
    con->ptr,
    kbd->device_id,
    affect_which, clear, select_all,
    affect_map, map, details
  );

  error = xcb_request_check(con->ptr, cookie);
  if (!error)
  {
    free(error);
    return false;
  }

  return true;
}


const char *
get_keyboard_layout_fullname(connection_t *con, keyboard_layout_t *kbd)
{
  const char *name;
  xcb_xkb_get_state_cookie_t cookie;
  xcb_xkb_get_state_reply_t *reply;

  if (!con || !con->ptr || !kbd)
    return NULL;

  cookie = xcb_xkb_get_state(con->ptr, kbd->device_id);

  reply = xcb_xkb_get_state_reply(con->ptr, cookie, NULL);
  if (!reply)
    return NULL;

  kbd->last_group = reply->lockedGroup;
  name = xkb_keymap_layout_get_name(kbd->keymap, reply->lockedGroup);
  free(reply);

  return name;
}


const char *
get_keyboard_layout_shortname(connection_t *con, keyboard_layout_t *kbd)
{
  static char buffer[3];
  const char *name;

  if (!con || !con->ptr || !kbd)
    NULL;

  buffer[0] = '?';
  buffer[1] = '?';
  buffer[2] = '\0';

  name = get_keyboard_layout_fullname(con, kbd);

  if (name && strlen(name) >= 2)
  {
    buffer[0] = tolower(name[0]);
    buffer[1] = tolower(name[1]);
  }

  return buffer;
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


bool
update_status(connection_t *con, keyboard_layout_t *kbd)
{
  char *status;
  const char *date;
  const char *lang;

  if (!con || !kbd)
    return false;

  date = get_datetime();
  lang = get_keyboard_layout_shortname(con, kbd);

  if (!date || !lang)
    return false;

  status = smprintf("%s %s", lang, date);
  if (!status)
    return false;

  if (!set_status(con, status))
    return false;

  free(status);
  return true;
}


bool
set_status(connection_t *con, const char *status)
{
  if (!con || !con->ptr || !con->screen || !status)
    return false;

  xcb_change_property(
    con->ptr,
    XCB_PROP_MODE_REPLACE,
    con->screen->root,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING, 8,
    strlen(status), status
  );

  xcb_flush(con->ptr);
  return true;
}
