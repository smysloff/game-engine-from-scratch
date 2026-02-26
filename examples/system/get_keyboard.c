
// file: examples/system/get_keyboard.c
// compile:

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>

xcb_connection_t *x11_init(void);
void x11_quit(xcb_connection_t *connection);
void x11_use_xkb(xcb_connection_t *connection);
void x11_unuse_xkb(struct xkb_context *context, struct xkb_keymap *keymap, struct xkb_state *state);
struct xkb_context *x11_xkb_get_context(xcb_connection_t *connection);
int32_t x11_xkb_get_device_id(xcb_connection_t *connection, struct xkb_context *context);
struct xkb_keymap *x11_xkb_get_keymap(xcb_connection_t *connection, struct xkb_context *context, int32_t device_id);
struct xkb_state *x11_xkb_get_state(xcb_connection_t *connection, struct xkb_context *context, int32_t device_id, struct xkb_keymap *keymap);


int
main(void)
{
  // variables
  char layout_name[256];

  // xcb
  xcb_connection_t *connection;

  // xkbcommon
  struct xkb_context *context;
  int32_t device;
  struct xkb_keymap *keymap;
  struct xkb_state *state;

  xkb_layout_index_t num_layouts;
  xkb_layout_index_t current_layout;
  const char *name;


  connection = x11_init();

  x11_use_xkb(connection);

  context = x11_xkb_get_context(connection);
  device  = x11_xkb_get_device_id(connection, context);
  keymap  = x11_xkb_get_keymap(connection, context, device);
  state   = x11_xkb_get_state(connection, context, device, keymap);


  // get current keyboard layout name

  num_layouts = xkb_keymap_num_layouts(keymap);
  current_layout = xkb_state_key_get_layout(state, XKB_STATE_LAYOUT_EFFECTIVE);

  if (current_layout != XKB_LAYOUT_INVALID)
  {
    name = xkb_keymap_layout_get_name(keymap, current_layout);

    (name == NULL)
      ? printf("layout: (%d of %lu)\n", current_layout, (unsigned long) num_layouts)
      : printf("layout: %s (%d of %lu)\n", name, current_layout, (unsigned long) num_layouts);
  }

  else
    printf("layout: unknown\n");


  // cleanup and exit

  x11_unuse_xkb(context, keymap, state);
  x11_quit(connection);

  return EXIT_SUCCESS;
}


xcb_connection_t *
x11_init(void)
{
  xcb_connection_t *connection;
  if (xcb_connection_has_error(connection))
  {
    fprintf(stderr, "Cannot connect to X-server\n");
    exit(EXIT_FAILURE);
  }
  return connection;
}

void
x11_quit(xcb_connection_t *connection)
{
  if (connection)
    xcb_disconnect(connection);
}

void
x11_use_xkb(xcb_connection_t *connection)
{
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  cookie = xcb_xkb_use_extension(
    connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  reply = xcb_xkb_use_extension_reply(connection, cookie, NULL);

  if (!reply || !reply->supported)
  {
    free(reply);
    fprintf(stderr, "error: xcb_xkb_use_extension_reply()\n");
    x11_quit(connection);
    exit(EXIT_FAILURE);
  }

  free(reply);
}

void
x11_unuse_xkb(
  struct xkb_context *context,
  struct xkb_keymap *keymap,
  struct xkb_state *state
) {
  xkb_state_unref(state);
  xkb_keymap_unref(keymap);
  xkb_context_unref(context);
}

struct xkb_context *
x11_xkb_get_context(xcb_connection_t *connection)
{
  struct xkb_context *context;
  if (!(context = xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
  {
    fprintf(stderr, "error: xkb_context_new()\n");
    x11_quit(connection);
    exit(EXIT_FAILURE);
  }
  return context;
}

int32_t
x11_xkb_get_device_id(xcb_connection_t *connection, struct xkb_context *context)
{
  int32_t id;
  if ((id = xkb_x11_get_core_keyboard_device_id(connection)) == -1)
  {
    fprintf(stderr, "error: xkb_x11_get_core_keyboard_device()\n");
    xkb_context_unref(context);
    xcb_disconnect(connection);
    exit(EXIT_FAILURE);
  }
  return id;
}

struct xkb_keymap *
x11_xkb_get_keymap(
  xcb_connection_t *connection,
  struct xkb_context *context,
  int32_t device_id
) {
  struct xkb_keymap *keymap;

  keymap = xkb_x11_keymap_new_from_device(
    context, connection, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!keymap)
  {
    fprintf(stderr, "error: xkb_x11_keymap_new_from_device()\n");
    xkb_context_unref(context);
    xcb_disconnect(connection);
    exit(EXIT_FAILURE);
  }

  return keymap;
}

struct xkb_state *
x11_xkb_get_state(
  xcb_connection_t *connection,
  struct xkb_context *context,
  int32_t device_id,
  struct xkb_keymap *keymap
) {
  struct xkb_state *state;
  if (!(state = xkb_x11_state_new_from_device(keymap, connection, device_id)))
  {
    fprintf(stderr, "error: xkb_x11_keymap_new_from_device\n");
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    xcb_disconnect(connection);
    exit(EXIT_FAILURE);
  }
  return state;
}
