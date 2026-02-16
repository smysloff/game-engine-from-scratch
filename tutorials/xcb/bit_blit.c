
// file: tutorials/xcb/bit_blit.c


#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include "core/core.h"


const i32_t WINDOW_WIDTH  = 990;
const i32_t WINDOW_HEIGHT = 540;
const i32_t WINDOW_SIZE   = WINDOW_WIDTH * WINDOW_HEIGHT;


void
gl_fill(i32_t *buffer, i32_t size, u32_t color)
{
  for (i32_t i = 0; i < size; ++i)
    buffer[i] = color;
}

void
gl_put_pixel(
  i32_t *buffer, i32_t width, i32_t height,
  i32_t x, i32_t y, u32_t color
) {
  if (x >= 0 && x < width && y >= 0 && y < height)
    buffer[y * width + x] = color;
}

void
gl_draw_line(
  i32_t *buffer, i32_t width, i32_t height,
  i32_t x0, i32_t y0, i32_t x1, i32_t y1, u32_t color
) {
  i32_t dx  =  absolute_number(x1 - x0);
  i32_t dy  = -absolute_number(y1 - y0);
  i32_t sx  =  x0 < x1 ? 1 : -1;
  i32_t sy  =  y0 < y1 ? 1 : -1;
  i32_t err =  dx + dy;

  gl_put_pixel(buffer, width, height, x0, y0, color);

  while (true)
  {
    gl_put_pixel(buffer, width, height, x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    i32_t err2 = err * 2;
    if (err2 >= dy) { err += dy; x0 += sx; }
    if (err2 <= dx) { err += dx; y0 += sy; }
  }
}


int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  i32_t *fbuffer;
  xcb_image_t *image;
  xcb_pixmap_t pixmap;
  xcb_gcontext_t gc;
  xcb_generic_event_t *event;


  // create connection
  connection = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;


  // create window
  window = xcb_generate_id(connection);

  xcb_create_window(
    connection,                            // connection
    XCB_COPY_FROM_PARENT,                  // depth (bpi)
    window,                                // window
    screen->root,                          // parent
    0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 1,  // geometry
    XCB_WINDOW_CLASS_INPUT_OUTPUT,         // class
    screen->root_visual,                   // visual

    XCB_CW_BACK_PIXEL                      // value mask
    | XCB_CW_EVENT_MASK,

    (u32_t[]) {                            // value list
      screen->black_pixel,
      XCB_EVENT_MASK_KEY_PRESS,
    }
  );


  // create frame buffer
  fbuffer = malloc(WINDOW_SIZE * sizeof(i32_t));

  image = xcb_image_create_native(
    connection,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    XCB_IMAGE_FORMAT_Z_PIXMAP,
    screen->root_depth,
    NULL, 0, (u8_t *) fbuffer
  );


  // create pixmap (server buffer)
  pixmap = xcb_generate_id(connection);

  xcb_create_pixmap(
    connection,
    screen->root_depth,
    pixmap,
    window,
    WINDOW_WIDTH, WINDOW_HEIGHT
  );

  // create gc
  gc = xcb_generate_id(connection);
  xcb_create_gc(connection, gc, pixmap, 0, NULL);


  // add class name (for Window Manager)
  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;
  const char atom_name[] = "WM_CLASS";
  const char atom_value[] = "xcbexample\0XCBExample\0";

  cookie = xcb_intern_atom(connection, 0, sizeof(atom_name) -1, atom_name);
  reply  = xcb_intern_atom_reply(connection, cookie, NULL);

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    sizeof(atom_value) - 1,
    atom_value
  );

  free(reply);


  // show window
  xcb_map_window(connection, window);
  xcb_flush(connection);


  // loop
  for (bool_t loop = true; loop; )
  {
    while ((event = xcb_poll_for_event(connection)))
    {
      switch (event->response_type & ~0x80)
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) event;
          if (key->detail == 9) loop = false;
          break;
      }
      free(event);
    }

    // draw
    gl_fill(fbuffer, WINDOW_SIZE, 0x000000);

    gl_draw_line(
      fbuffer, WINDOW_WIDTH, WINDOW_HEIGHT,                 // buffer data
      0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, 0x0000FF   // coords and color
    );

    gl_draw_line(
      fbuffer, WINDOW_WIDTH, WINDOW_HEIGHT,                 // buffer data
      0, WINDOW_HEIGHT - 1, WINDOW_WIDTH - 1, 0, 0xFF0000   // coords and color
    );

    gl_put_pixel(
      fbuffer, WINDOW_WIDTH, WINDOW_HEIGHT,                 // buffer data
      WINDOW_WIDTH / 2 - 1, WINDOW_HEIGHT / 2 - 1, 0x00FF00 // coords and color
    );


    // bit blit
    xcb_image_put(
      connection,
      pixmap,
      gc,
      image,
      0, 0, 0 // x, y, left_pad
    );

    xcb_copy_area(
      connection,
      pixmap,
      window,
      gc,
      0, 0,
      0, 0,
      WINDOW_WIDTH, WINDOW_HEIGHT
    );


    // update window
    xcb_flush(connection);
  }


  // close connection and cleanup
  xcb_free_gc(connection, gc);
  xcb_free_pixmap(connection, pixmap);
  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);
  if (fbuffer) free(fbuffer);
  if (image) free(image);
}
