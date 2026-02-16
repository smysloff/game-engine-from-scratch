
// tutorials/xcb/open_window.c

// Simple example of creating a window in XCB
// - creates empty window
// - shows it for some time

#include <xcb/xcb.h>
#include "core/core.h"

int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_t     *screen;
  xcb_window_t      window;

  // establish a connection to the X server
  // and check for connection errors
  connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(connection))
    return 1; // terminate the program if there is a connection error

  // get the first available screen
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  // generate a unique identifier for the window
  window = xcb_generate_id(connection);

  // create the window with basic configuration
  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,
    window,
    screen->root, // parent window (root of the screen)
    0, 0, 990, 540, // position (x, y) and size (width, height)
    1, // border width
    XCB_WINDOW_CLASS_INPUT_OUTPUT, // window class
    screen->root_visual, // visual type
    0, NULL // mask and values not used in this example
  );

  // map the window to make it visible on the screen
  xcb_map_window(connection, window);
  xcb_flush(connection); // synchronize client and server

  // wait for 5 seconds to make the window visible
  print_log_endl("wait for 5 seconds");
  sleep(5);

  // cleanup and disconnect from the X server
  xcb_disconnect(connection);

  return 0;
}
