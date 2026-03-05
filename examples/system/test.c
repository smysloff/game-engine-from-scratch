
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
close_connection(xcb_connection_t *con)
{
  if (con)
    xcb_disconnect(con);
}


xcb_connection_t *
create_connection(bool use_kbd_ext)
{
  xcb_connection_t *con;

  con = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(con))
  {
    close_connection(con);
    return NULL;
  }

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

    free()
  }
}

keyboard_layout_t *
get_keyboard_layout(xcb_connection_t *con)
{

}


int
main(void)
{
  xcb_connection_t *con;

  con = create_connection(true);
  if (!con)
  {
    perror("Cannot create connection to X-Server and/or use XKB extension");
    close_connection(con);
    return EXIT_FAILURE;
  }

  printf("It works!\n");

  close_connection(con);
  return EXIT_SUCCESS;
}
