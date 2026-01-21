
// file: examples/xcb/draw_api.c

#include <xcb/xcb.h>
#include "core/core.h"

xcb_point_t points[] =
{
  { 10, 10 },
  { 20, 10 },
  { 20, 20 },
  { 10, 20 },
};

xcb_point_t polyline[] =
{
  { 50,  10 },
  {  5,  20 }, /* rest of points are relative */
  { 25, -20 },
  { 10,  10 },
};

xcb_segment_t segments[] =
{
  { 100, 10, 140, 30 },
  { 110, 25, 130, 60 },
};

xcb_rectangle_t rectangles[] =
{
  { 10, 50, 40, 20 },
  { 80, 50, 10, 40 },
};

xcb_arc_t arcs[] =
{
  { 10, 100, 60, 40, 0,  90 << 6 },
  { 90, 100, 55, 40, 0, 270 << 6 }
};


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
  xcb_gcontext_t gcontext;
  bool_t loop;
} gl_t;


void
gl_create_window(gl_t *gl, uint16_t width, uint16_t height)
{
  uint32_t value_mask;
  uint32_t value_list[2];

  gl->connection = xcb_connect(NULL, NULL);
  gl->screen = xcb_setup_roots_iterator(xcb_get_setup(gl->connection)).data;
  gl->window = xcb_generate_id(gl->connection);

  value_mask = XCB_CW_BACK_PIXEL
             | XCB_CW_EVENT_MASK;

  value_list[0] = gl->screen->black_pixel;

  value_list[1] = XCB_EVENT_MASK_EXPOSURE
                | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_window(
    gl->connection,                // connection
    XCB_COPY_FROM_PARENT,          // depth
    gl->window,                    // window id
    gl->screen->root,              // parent window
    0, 0, width, height, 1,        // pos, size, border
    XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
    gl->screen->root_visual,       // visual
    value_mask, value_list         // masks
  );

  xcb_map_window(gl->connection, gl->window);
  xcb_flush(gl->connection);
}

void
gl_destroy_window(gl_t *gl)
{
  xcb_disconnect(gl->connection);
  gl->connection = NULL;
  gl->screen = NULL;
}

void
gl_free_event(gl_t *gl)
{
  if (gl->event != NULL)
  {
    free(gl->event);
    gl->event = NULL;
  }
}

void
gl_set_color(gl_t *gl, uint32_t fgcolor, uint32_t bgcolor)
{
  uint32_t value_mask;
  uint32_t value_list[2];

  value_mask = XCB_GC_FOREGROUND
             | XCB_GC_BACKGROUND;

  value_list[0] = fgcolor;
  value_list[1] = bgcolor;

  gl->gcontext = xcb_generate_id(gl->connection);

  xcb_create_gc(
    gl->connection,
    gl->gcontext,
    gl->window,
    value_mask, value_list
  );
}

void
expose_handler(gl_t *gl)
{
  xcb_expose_event_t *e = (xcb_expose_event_t *) gl->event;

  print_string("log: expose window ");
  print_number(e->window);
  print_endl();

  print_string("log: region to be redrawn at location (");
  print_number(e->x); print_char(','); print_number(e->y); print_string("), ");
  print_string("width dimension (");
  print_number(e->width); print_char(','); print_number(e->height);
  print_string(")");
  print_endl();
}

void
key_press_handler(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_string("log: pressed key with code: ");
  print_number(e->detail);
  print_endl();

  if (e->detail == KEY_ESC)
    gl->loop = false;
}

void
draw_frame(gl_t *gl)
{
  xcb_clear_area(
    gl->connection,
    0,
    gl->window,
    0, 0,
    gl->screen->width_in_pixels,
    gl->screen->height_in_pixels
  );

  xcb_poly_point(
    gl->connection,
    XCB_COORD_MODE_ORIGIN,
    gl->window,
    gl->gcontext,
    4, points
  );

  xcb_poly_line(
    gl->connection,
    XCB_COORD_MODE_PREVIOUS,
    gl->window,
    gl->gcontext,
    4, polyline
  );

  xcb_poly_segment(
    gl->connection,
    gl->window,
    gl->gcontext,
    2, segments
  );

  xcb_poly_rectangle(
    gl->connection,
    gl->window,
    gl->gcontext,
    2, rectangles
  );

  xcb_poly_arc(
    gl->connection,
    gl->window,
    gl->gcontext,
    2, arcs
  );

  xcb_flush(gl->connection);
}

void
loop_frames(gl_t *gl)
{
  draw_frame(gl);
  print_log("press ESC key to exit");

  for (gl->loop = true; gl->loop; )
  {
    while ((gl->event = xcb_poll_for_event(gl->connection)))
    {
      switch (gl->event->response_type & ~0x80)
      {
        case XCB_EXPOSE:
          expose_handler(gl);
          draw_frame(gl);
          print_log("press ESC key to exit");
          break;

        case XCB_KEY_PRESS:
          key_press_handler(gl);
          print_log("press ESC key to exit");
          break;
      }
    }
    gl_free_event(gl);
  }
}

int
main(void)
{
  gl_t gl;

  gl_create_window(&gl, 640, 480);
  gl_set_color(&gl, gl.screen->white_pixel, gl.screen->black_pixel);
  loop_frames(&gl);
  gl_destroy_window(&gl);

  return 0;
}
