
// file: examples/xcb/test.c


#include <xcb/xcb_image.h>
#include "core/core.h"


int
main(void)
{

  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_gcontext_t gc; unused(gc);
  u8_t *buffer;
  xcb_image_t *immage;
  xcb_generic_event_t *event;


  // create connection
  connection = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;


  // create window
  window = xcb_generate_id(connection);

  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,
    window,
    screen->root,
    0, 0, 990, 540, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    XCB_CW_BACK_PIXEL,
    (u32_t[]) { screen->black_pixel }
  );


  // show window
  xcb_map_window(connection, window);
  xcb_flush(connection);


  // loop
  sleep(3);


  // close connection
  xcb_disconnect(connection);
}
