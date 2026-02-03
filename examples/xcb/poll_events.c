
// file: examples/xcb/poll_events.c

#include <xcb/xcb.h>
#include "core/core.h"

#ifdef __APPLE__
  #define KEY_ESC 41 // MacOS HID
#else
  #define KEY_ESC 9  // Linux X11
#endif

typedef struct
{
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_generic_event_t *event;
  bool_t loop;
} gl_t;

void
expose_handler(gl_t *gl)
{
  xcb_expose_event_t *e = (xcb_expose_event_t *) gl->event;

  print_format_endl("log: expose window %d", e->window);

  print_format_endl(
    "log: region to be redrawn at location (%d,%d), width dimension (%d,%d)",
      e->x, e->y, e->width, e->height);

  print_log_endl("press ESC key to exit");
}

void
key_press_handler(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_format_endl("log: pressed key with code: %d", e->detail);

  if (e->detail == KEY_ESC)
    gl->loop = false;

  print_log_endl("press ESC key to exit");
}

int
main(void)
{
  gl_t gl;

  uint32_t cw_window_mask;
  uint32_t cw_window_list[2];

  gl.connection = xcb_connect(NULL, NULL);
  gl.screen = xcb_setup_roots_iterator(xcb_get_setup(gl.connection)).data;
  gl.window = xcb_generate_id(gl.connection);

  cw_window_mask = XCB_CW_BACK_PIXEL
                 | XCB_CW_EVENT_MASK;

  cw_window_list[0] = gl.screen->black_pixel;

  cw_window_list[1] = XCB_EVENT_MASK_EXPOSURE
                    | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_window(
    gl.connection,
    XCB_COPY_FROM_PARENT,
    gl.window,
    gl.screen->root,
    0, 0, 640, 480, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    gl.screen->root_visual,
    cw_window_mask, cw_window_list
  );

  xcb_map_window(gl.connection, gl.window);
  xcb_flush(gl.connection);

  print_log_endl("press ESC key to exit");

  for (gl.loop = true; gl.loop; )                          // =============
  {                                                        // <-- GAME LOOP
    while ((gl.event = xcb_poll_for_event(gl.connection))) // =============
    {
      switch (gl.event->response_type & ~0x80)
      {
        case XCB_EXPOSE: expose_handler(&gl);
          break;

        case XCB_KEY_PRESS: key_press_handler(&gl);
          break;
      }
    }
    free(gl.event);
    gl.event = NULL;
  }

  xcb_disconnect(gl.connection);

  gl.connection = NULL;
  gl.screen = NULL;

  return 0;
}
