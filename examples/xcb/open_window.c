
// examples/xcb/01.c

#include <xcb/xcb.h>
#include "core/core.h"

int main(void)
{
  xcb_connection_t *connection = xcb_connect(NULL, NULL);

  xcb_screen_t *screen = xcb_setup_roots_iterator(
    xcb_get_setup(connection)).data;

  xcb_window_t window = xcb_generate_id(connection);

  xcb_create_window(
    connection, XCB_COPY_FROM_PARENT, window, screen->root, // connection, depth, window, parent
    0, 0, 640, 480, 1,                                      // coords, sizes, border
    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,     // class, visual
    0, NULL                                                 // mask, values
  );

  xcb_map_window(connection, window);
  xcb_flush(connection);

  sleep(5);

  xcb_disconnect(connection);

  return 0;
}
