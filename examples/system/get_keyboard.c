
// file: examples/system/get_keyboard.c

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>


typedef struct x11_xkb_layout_s
{
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  int32_t device_id;
} x11_xkb_layout_t;


xcb_connection_t *
x11_init(void)
{
  xcb_connection_t *connection;
  connection = xcb_connect(NULL, NULL);
  assert(!xcb_connection_has_error(connection));
  return connection;
}

void
x11_quit(xcb_connection_t *connection)
{
  assert(connection);
  xcb_disconnect(connection);
}

void
x11_use_xkb(xcb_connection_t *connection)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  assert(connection);

  cookie = xcb_xkb_use_extension(connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  reply = xcb_xkb_use_extension_reply(connection, cookie, NULL);

  assert(reply && reply->supported);
  free(reply);
}

x11_xkb_layout_t *
x11_xkb_layout_init(xcb_connection_t *connection)
{
  x11_xkb_layout_t *layout;

  assert(connection);

  layout = malloc(sizeof(x11_xkb_layout_t));
  assert(layout);

  layout->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  assert(layout->context);

  layout->device_id = xkb_x11_get_core_keyboard_device_id(connection);
  assert(layout->device_id != -1);

  layout->keymap = xkb_x11_keymap_new_from_device(
    layout->context, connection, layout->device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
  assert(layout->keymap);

  layout->state = xkb_x11_state_new_from_device(
    layout->keymap, connection, layout->device_id);
  assert(layout->state);

  return layout;
}

void
x11_xkb_layout_free(x11_xkb_layout_t *layout)
{
  assert(layout);
  xkb_state_unref(layout->state);
  xkb_keymap_unref(layout->keymap);
  xkb_context_unref(layout->context);
  free(layout);
}

const char *
x11_xkb_layout_get_fullname(
  xcb_connection_t *connection,
  x11_xkb_layout_t *layout
) {
  const char *fullname;
  xcb_xkb_get_state_cookie_t cookie;
  xcb_xkb_get_state_reply_t *reply;

  assert(connection);
  assert(layout);

  cookie = xcb_xkb_get_state(connection, layout->device_id);
  reply = xcb_xkb_get_state_reply(connection, cookie, NULL);
  assert(reply);
  fullname = xkb_keymap_layout_get_name(layout->keymap, reply->lockedGroup);
  free(reply);

  return fullname;
}

const char *
x11_xkb_layout_get_shortname(
  xcb_connection_t *connection,
  x11_xkb_layout_t *layout
) {
  static char shortname[3];
  const char *fullname;

  assert(connection);
  assert(layout);

  fullname = x11_xkb_layout_get_fullname(connection, layout);

  if (fullname && strlen(fullname) >= 2)
  {
    shortname[0] = tolower(fullname[0]);
    shortname[1] = tolower(fullname[1]);
  }
  else
  {
    shortname[0] = '?';
    shortname[1] = '?';
  }
  shortname[2] = '\0';

  return shortname;
}


int
main(void)
{
  xcb_connection_t *connection;
  x11_xkb_layout_t *layout;
  const char *layout_shortname;
  const char *layout_fullname;

  connection = x11_init();
  x11_use_xkb(connection);
  layout = x11_xkb_layout_init(connection);

  layout_shortname = x11_xkb_layout_get_shortname(connection, layout);
  layout_fullname = x11_xkb_layout_get_fullname(connection, layout);

  printf("current layout: %s (%s)\n",
    layout_shortname, layout_fullname ? layout_fullname : "Unknown");

  x11_xkb_layout_free(layout);
  x11_quit(connection);

  return EXIT_SUCCESS;
}
