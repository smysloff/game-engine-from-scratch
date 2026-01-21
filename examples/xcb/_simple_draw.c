
// file: examples/xcb/simple_draw.c

//
// Include headers
//

#include <stdio.h>  // printf
#include <stdlib.h> // free

#include <xcb/xcb.h>

#define CORE_IMPLEMENTATION
#include "core/core.h"

void draw_frame(

  xcb_connection_t *connection,
  const xcb_screen_t *screen,
  xcb_window_t window,
  xcb_gcontext_t gcontext,

  const xcb_point_t *points,
  usize_t points_count,

  const xcb_rectangle_t *rectangles,
  usize_t rectangles_count

) {
  xcb_clear_area(
    connection,
    0,
    window,
    0, 0,
    screen->width_in_pixels,
    screen->height_in_pixels
  );

  //xcb_poly_point(
  //  connection,
  //  XCB_COORD_MODE_ORIGIN,
  //  window,
  //  gcontext,
  //  points_count,
  //  points
  //);

  xcb_poly_line(
    connection,
    XCB_COORD_MODE_ORIGIN,
    window,
    gcontext,
    points_count,
    points
  );

  xcb_poly_rectangle(
    connection,
    window,
    gcontext,
    rectangles_count,
    rectangles
  );
}

int
main(void)
{
  xcb_connection_t *connection;

  xcb_screen_t *screen;
  i32_t screen_number;
  xcb_screen_iterator_t iterator;

  xcb_atom_t atom;
  xcb_intern_atom_cookie_t cookie;
  xcb_intern_atom_reply_t *reply;

  xcb_window_t window;

  xcb_gcontext_t gcontext;

  const xcb_point_t points[] = {
    { .x =  50, .y =  50 },
    { .x = 250, .y =  50 },
    { .x = 250, .y = 250 },
    { .x =  50, .y = 250 },
  };

  const xcb_rectangle_t rectangles[] = {
    { .x = 100, .y = 100, .width = 200, .height = 200 },
  };


  // Create connection to X11

  connection = xcb_connect(NULL, &screen_number);
  if (!connection)
  {
    print_error("xcb_connect()");
    return -1;
  }


  // Get screen

  screen = NULL;
  iterator = xcb_setup_roots_iterator(xcb_get_setup(connection));

  for (; iterator.rem; --screen_number, xcb_screen_next(&iterator))
  {
    if (screen_number == 0)
    {
      screen = iterator.data;
      break;
    }
  }

  // ^alternative way:
  // screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  if (!screen)
  {
    print_error("xcb_screen_next()");
    return -1;
  }

  printf("\n");
  printf("Informations of screen %u:\n", screen->root);
  printf("  width.........: %d\n",       screen->width_in_pixels);  // 1920
  printf("  height........: %d\n",       screen->height_in_pixels); // 1080
  printf("  white pixel...: #%06x\n",    screen->white_pixel);      // #ffffff
  printf("  black pixel...: #%06x\n",    screen->black_pixel);      // #000000
  printf("\n");


  // Atom request/reply example

  char prop[] = "_NET_WM_NAME";

  cookie = xcb_intern_atom(connection, 0, string_length(prop), prop);
  reply = xcb_intern_atom_reply(connection, cookie, NULL);
  atom = reply->atom;

  printf("NET_WN_NAME ID: %u\n", atom);

  free(reply);
  reply = NULL;


  // Create window

  uint32_t cw_value_mask = XCB_CW_BACK_PIXEL
                         | XCB_CW_EVENT_MASK;

  uint32_t cw_value_list[] = {
    screen->black_pixel,
    XCB_EVENT_MASK_EXPOSURE,
  };

  window = xcb_generate_id(connection);

  xcb_create_window(
    connection,                    // connection,
    XCB_COPY_FROM_PARENT,          // depth (same as root),
    window,                        // window id,
    screen->root,                  // parent window,
    0, 0,                          // x, y,
    screen->width_in_pixels,       // width
    screen->height_in_pixels,      // height
    0,                             // border_width,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, // _class,
    screen->root_visual,           // visual,
    cw_value_mask, cw_value_list   // value_mask, value_list
  );

  xcb_map_window(connection, window);


  // Create Graphic Context

  u32_t gc_value_mask = XCB_GC_FOREGROUND
                      | XCB_GC_BACKGROUND;

  const u32_t gc_value_list[] = {
    screen->white_pixel,
    screen->black_pixel,
  };

  gcontext = xcb_generate_id(connection);

  xcb_create_gc(
    connection,
    gcontext,
    window,
    gc_value_mask,
    gc_value_list
  );


  // Draw examples

  draw_frame(

    connection,
    screen,
    window,
    gcontext,

    points,
    sizeof(points) / sizeof(*points),

    rectangles,
    sizeof(rectangles) / sizeof(*rectangles)

  );


  // Expose window and all drawings

  xcb_flush(connection);


  // Don't close window

  while (true)
  {
    xcb_generic_event_t *event = xcb_wait_for_event(connection);
    if (!event) break;

    switch (event->response_type & ~0x80)
    {
      case XCB_EXPOSE:
      {

        draw_frame(

          connection,
          screen,
          window,
          gcontext,

          points,
          sizeof(points) / sizeof(*points),

          rectangles,
          sizeof(rectangles) / sizeof(*rectangles)

        );

        xcb_flush(connection);

        break;
      }
      default: break;
    }
  }


  // Disconnect from X11

  xcb_disconnect(connection);

  screen = NULL;
  connection = NULL;


  // Exit

  return 0;
}
