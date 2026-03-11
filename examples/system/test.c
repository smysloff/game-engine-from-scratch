
// file: examples/system/test.c

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>


int
cleanup(
  const char *error_msg,
  xcb_connection_t *conn,
  struct xkb_context *ctx,
  struct xkb_keymap *keymap,
  struct xkb_state *state
) {
  if (error_msg) perror(error_msg);
  if (state) xkb_state_unref(state);
  if (keymap) xkb_keymap_unref(keymap);
  if (ctx) xkb_context_unref(ctx);
  if (conn) xcb_disconnect(conn);
  return error_msg ? EXIT_FAILURE : EXIT_SUCCESS;
}


int
main(void)
{
  xcb_connection_t *conn;
  struct xkb_context *ctx;
  struct xkb_keymap *keymap;
  struct xkb_state *state;
  int32_t device_id;


  // Create connection to X-Server

  conn = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(conn))
    return cleanup("Connection get connection to X-Server", NULL, NULL, NULL, NULL);


  // Use XKB extension

  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;

  cookie = xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  if (!(reply = xcb_xkb_use_extension_reply(conn, cookie, NULL)))
    return cleanup("Cannot use XKB extension", conn, NULL, NULL, NULL);

  free(reply);


  // Init XKB variables

  if (!(ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
    return cleanup("Cannot get XKB context", conn, NULL, NULL, NULL);

  if (!(device_id = xkb_x11_get_core_keyboard_device_id(conn)))
    return cleanup("Cannot get XKB device_id", conn, NULL, NULL, NULL);

  if (!(keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS)))
    return cleanup("Cannot get XKB keymap", conn, ctx, NULL, NULL);

  if (!(state = xkb_x11_state_new_from_device(keymap, conn, device_id)))
    return cleanup("Cannot get XKB state", conn, ctx, keymap, NULL);


  // Get keymap layout name

  char kbd[3];
  const char *layout;

  while (1)
  {
    xcb_xkb_get_state_cookie_t cookie;
    xcb_xkb_get_state_reply_t *reply;

    cookie = xcb_xkb_get_state(conn, device_id);

    if (!(reply = xcb_xkb_get_state_reply(conn, cookie, NULL)))
      return cleanup("Cannot get xcb_xkb_get_state_reply", conn, ctx, keymap, state);

    if (!(layout = xkb_keymap_layout_get_name(keymap, reply->lockedGroup)))
    {
      free(reply);
      return cleanup("Cannot get layout name", conn, ctx, keymap, state);
    }

    memset(kbd, '\0', sizeof(kbd));
    if (strlen(layout) >= 2)
    {
      kbd[0] = tolower(layout[0]);
      kbd[1] = tolower(layout[1]);
    } else {
      memset(kbd, '?', 2);
    }

    printf("kbd: %s\n", kbd);
    free(reply);

    xcb_flush(conn);
    xcb_wait_for_event(conn);
  }


  // Cleanup and exit

  return cleanup(NULL, conn, ctx, keymap, state);
}
