
// examples/xcb/wait_events.c

#include <xcb/xcb.h>
#include "core/core.h"

#ifdef __APPLE__
  #define KEY_ESC 41 // MacOS HID
#else
  #define KEY_ESC 9  // Linux X11
#endif

int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_generic_event_t *event;

  u32_t cw_window_mask;
  u32_t cw_window_list[2];

  connection = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  window = xcb_generate_id(connection);

  cw_window_mask = XCB_CW_BACK_PIXEL
                 | XCB_CW_EVENT_MASK;

  cw_window_list[0] = screen->black_pixel;
  cw_window_list[1] = XCB_EVENT_MASK_EXPOSURE
                    | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_window(
    connection,                    // connection
    XCB_COPY_FROM_PARENT,          // depth
    window,                        // window
    screen->root,                  // parent
    0, 0, 640, 480, 1,             // coords, sizes, border
    XCB_WINDOW_CLASS_INPUT_OUTPUT, // _class
    screen->root_visual,           // visual
    cw_window_mask, cw_window_list
  );

  xcb_map_window(connection, window);
  xcb_flush(connection);

  print_log_endl("press ESC key to exit");

  for (bool_t loop = true; loop && (event = xcb_wait_for_event(connection)); )
  {
    switch (event->response_type & ~0x80)
    {
      case XCB_EXPOSE:
      {
        xcb_expose_event_t *e = (xcb_expose_event_t *) event;

        //print_string("log: expose window ");
        //print_number(e->window);
        //print_endl();

        print_format_endl("log: expose window %d", e->window);

        //print_string("log: region to be redrawn at location (");
        //print_number(e->x); print_char(','); print_number(e->y); print_string("), ");
        //print_string("width dimension (");
        //print_number(e->width); print_char(','); print_number(e->height);
        //print_string(")");
        //print_endl();

        print_format_endl(
          "log: region to be redrawn at location (%d,%d), width dimension (%d,%d)",
            e->x, e->y, e->width, e->height);

        print_log_endl("press ESC key to exit");

        break;
      }

      case XCB_KEY_PRESS:
      {
        xcb_key_press_event_t *e = (xcb_key_press_event_t *) event;

        //print_string("log: pressed key with code: ");
        //print_number(e->detail);
        //print_endl();

        print_format_endl("log: pressed key with code: %d", e->detail);

        if (e->detail == KEY_ESC)
          loop = false;

        print_log_endl("press ESC key to exit");

        break;
      }

    }

    free(event);
    event = NULL;
  }

  xcb_disconnect(connection);

  connection = NULL;
  screen = NULL;

  return 0;
}
