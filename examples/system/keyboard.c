
// file: examples/system/keyboard.c
// see: https://xkbcommon.org/doc/current/md_doc_2quick-guide.html
// compile with xcb to work with X-server:
// gcc keyboard.c `pkg-config --cflags --libs xcb xcb-xkb xkbcommon-x11`

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>

int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_iterator_t screen_iterator;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_generic_event_t *event;
  uint32_t mask;
  uint32_t values[2];
  struct xkb_context *ctx;
  int32_t device_id;
  xcb_xkb_use_extension_cookie_t cookie;
  xcb_xkb_use_extension_reply_t *reply;
  struct xkb_keymap *keymap;
  struct xkb_state *state;


  // connection to X-server

  connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(connection))
  {
    fprintf(stderr, "error: xcb_connect\n");
    return EXIT_FAILURE;
  }


  // use XKB extension

  cookie = xcb_xkb_use_extension(connection,
    XCB_XKB_MAJOR_VERSION,
    XCB_XKB_MINOR_VERSION);

  reply = xcb_xkb_use_extension_reply(connection, cookie, NULL);

  if (!reply || !reply->supported)
  {
    fprintf(stderr, "error: xcb_xkb_use_extension_reply\n");
    free(reply);
    return EXIT_FAILURE;
  }

  free(reply);


  // get XKB context

  ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!ctx)
  {
    fprintf(stderr, "error: xkb_context_new\n");
    return EXIT_FAILURE;
  }


  // get keyboard device_id

  device_id = xkb_x11_get_core_keyboard_device_id(connection);
  if (device_id == -1)
  {
    fprintf(stderr, "error: xkb_x11_get_core_keyboard_device_id\n");
    return EXIT_FAILURE;
  }


  // get keyboard keymap

  keymap = xkb_x11_keymap_new_from_device(ctx, connection, device_id,
    XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!keymap)
  {
    fprintf(stderr, "error: xkb_keymap_new_from_device\n");
    return EXIT_FAILURE;
  }


  // get keyboad state

  state = xkb_x11_state_new_from_device(keymap, connection, device_id);
  if (!state)
  {
    fprintf(stderr, "error: xkb_x11_state_new_from_device\n");
    return EXIT_FAILURE;
  }


  // create and config window
  // register keyboard events

  screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(connection));
  screen = screen_iterator.data;
  window = xcb_generate_id(connection);

  mask      = XCB_CW_BACK_PIXEL
            | XCB_CW_EVENT_MASK;
  values[0] = screen->black_pixel;
  values[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;

  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,
    window,
    screen->root,
    0, 0, 640, 480, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    mask, values
  );

  const char wm_class[] = "xcb_example\0xcb_example\0";
  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    XCB_ATOM_WM_CLASS,
    XCB_ATOM_STRING,
    8,
    sizeof(wm_class),
    wm_class
  );

  xcb_map_window(connection, window);
  xcb_set_input_focus(connection, XCB_INPUT_FOCUS_PARENT, window, XCB_CURRENT_TIME);
  xcb_flush(connection);


  // event loop

  while ((event = xcb_wait_for_event(connection)))
  {
    switch (event->response_type & ~0x80)
    {
      case XCB_KEY_PRESS:
      {
        char buffer[8];
        int size;

        xcb_key_press_event_t *key = (xcb_key_press_event_t *) event;
        xkb_keycode_t keycode = key->detail;
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(state, keycode);
        size = xkb_state_key_get_utf8(state, keycode, buffer, sizeof(buffer));

        // example:
        // Key press - keycode: 47, keysym: 0x3b (semicolon), utf8: ';'

        printf("Key press - keycode: %d", keycode);

        if (keysym != XKB_KEY_NoSymbol)
        {
          char name[64];
          printf(", keysym: 0x%x", keysym);
          xkb_keysym_get_name(keysym, name, sizeof(name));
          printf(" (%s)", name);
        }

        if (size > 0 && buffer[0] != '\0')
        {
          printf(", utf8: '%s'", buffer);
        }

        printf("\n");

        if (keysym == XKB_KEY_Escape)
        {
          free(event);
          goto cleanup;
        }
        break;
      }

      case XCB_DESTROY_NOTIFY:
      {
        printf("Window destroyed\n");
        free(event);
        goto cleanup;
      }

    }

    free(event);
  }


  // cleanup

cleanup:
  xkb_state_unref(state);
  xkb_keymap_unref(keymap);
  xkb_context_unref(ctx);
  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);


  // EXIT SUCCESS

  puts("EXIT_SUCCESS");
  return EXIT_SUCCESS;
}
