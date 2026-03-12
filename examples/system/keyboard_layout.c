
// file: examples/system/keyboard_layout.c

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

  xcb_xkb_use_extension_cookie_t ext_cookie;
  xcb_xkb_use_extension_reply_t *ext_reply;

  ext_cookie = xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

  if (!(ext_reply = xcb_xkb_use_extension_reply(conn, ext_cookie, NULL)))
    return cleanup("Cannot use XKB extension", conn, NULL, NULL, NULL);

  free(ext_reply);


  // Init XKB variables

  if (!(ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
    return cleanup("Cannot get XKB context", conn, NULL, NULL, NULL);

  if (!(device_id = xkb_x11_get_core_keyboard_device_id(conn)))
    return cleanup("Cannot get XKB device_id", conn, NULL, NULL, NULL);

  if (!(keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS)))
    return cleanup("Cannot get XKB keymap", conn, ctx, NULL, NULL);

  if (!(state = xkb_x11_state_new_from_device(keymap, conn, device_id)))
    return cleanup("Cannot get XKB state", conn, ctx, keymap, NULL);


  // Subscribe to XKB events

  xcb_xkb_select_events(
    conn,
    device_id,
    XCB_XKB_EVENT_TYPE_STATE_NOTIFY, // affectWhich мы хотим получать
    0,                               // clear какие события отключаем
    XCB_XKB_EVENT_TYPE_STATE_NOTIFY, // selectAll на какие события подписваемся
    0, 0, 0                          // affectMap, map, details
  );

  xcb_flush(conn);


  // Get keyboard layout name on start

  char keyboard_short[3];
  const char *keyboard_name;
  uint8_t keyboard_group;

  xcb_xkb_get_state_cookie_t state_cookie;
  xcb_xkb_get_state_reply_t *state_reply;

  state_cookie = xcb_xkb_get_state(conn, device_id);

  if (!(state_reply = xcb_xkb_get_state_reply(conn, state_cookie, NULL)))
    return cleanup("Cannot get xcb_xkb_get_state_reply", conn, ctx, keymap, state);

  keyboard_group = state_reply->lockedGroup;
  free(state_reply);

  // update keyboard (short) name
  if (!(keyboard_name = xkb_keymap_layout_get_name(keymap, keyboard_group)))
    return cleanup("Cannot get layout name", conn, ctx, keymap, state);

  memset(keyboard_short, '\0', sizeof(keyboard_short));

  if (strlen(keyboard_name) >= 2)
    for (size_t i = 0; i < 2; ++i)
      keyboard_short[i] = tolower(keyboard_name[i]);
  else
    memset(keyboard_short, '?', 2);

  // print keyboard (short) name
  printf("keyboard: %s\n", keyboard_short);


  // Get keyboard layout name on change

  const xcb_query_extension_reply_t *xkb_ext;
  xcb_generic_event_t *ev;
  xcb_xkb_state_notify_event_t *xkb_ev;
  uint8_t response_type;

  xkb_ext = xcb_get_extension_data(conn, &xcb_xkb_id);
  if (!xkb_ext || !xkb_ext->present)
    return cleanup("XKB extension not present", conn, ctx, keymap, state);

  while ((ev = xcb_wait_for_event(conn)))
  {
    response_type = ev->response_type & ~0x80;

    if (response_type == xkb_ext->first_event)
    {
      xkb_ev = (xcb_xkb_state_notify_event_t *) ev;

      if (xkb_ev->xkbType == XCB_XKB_STATE_NOTIFY)
      {

        if (keyboard_group == xkb_ev->group)
          continue;

        keyboard_group = xkb_ev->group;

        // update keyboard (short) name
        if (!(keyboard_name = xkb_keymap_layout_get_name(keymap, keyboard_group)))
          return cleanup("Cannot get layout name", conn, ctx, keymap, state); // @todo free(ev)

        memset(keyboard_short, '\0', sizeof(keyboard_short));

        if (strlen(keyboard_name) >= 2)
          for (size_t i = 0; i < 2; ++i)
            keyboard_short[i] = tolower(keyboard_name[i]);
        else
          memset(keyboard_short, '?', 2);

        // print keyboard (short) name
        printf("keyboard: %s\n", keyboard_short);
      }
    }

    free(ev);
  }


  // Cleanup and exit

  return cleanup(NULL, conn, ctx, keymap, state);
}
