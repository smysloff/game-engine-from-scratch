#include <xcb/xcb.h>
//#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *



int
main(void)
{
  xcb_connection_t  *connection;
  xcb_screen_t      *screen;
  struct xkb_keymap *keymap;

  connection = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;



  xcb_disconnect(connection);
  return EXIT_SUCCESS;
}
