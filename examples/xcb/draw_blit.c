
// file: examples/xcb/draw_blit.c


#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include "core/core.h"


const i32_t WINDOW_WIDTH  = 990;
const i32_t WINDOW_HEIGHT = 540;
const i32_t WINDOW_SIZE   = WINDOW_WIDTH * WINDOW_HEIGHT;


// Key codes

#if defined(__linux__) || defined(__unix__) || defined(__gnu_linux__)
  #define KEY_ESC 9  // Linux/Unix X11
#elif defined(__APPLE__)
  #define KEY_ESC 41 // MacOS HID
#elif defined(_WIN32) || defined(_WIN64)
  #define KEY_ESC 27 // Windows VK_ESCAPE or ASCII (0x1B)
#else
  perror("usuported platform");
  exit(-1);
#endif


typedef struct
{
  xcb_connection_t     *connection;
  xcb_screen_t         *screen;
  xcb_generic_event_t  *event;
  xcb_pixmap_t          pixmap;
  bool_t                loop;
  //u64_t                 frame;
} gl_context_t;

typedef struct
{
  xcb_window_t    id;
  xcb_gcontext_t  gc;
  i32_t           width, height;
  i32_t           byte_size;
  i32_t          *frame_buffer;
  xcb_image_t    *image;
  xcb_atom_t      wm_delete_atom;
} gl_window_t;

typedef u32_t color_t;


void
gl_init(gl_context_t *gl)
{
  assert(gl);

  gl->connection = xcb_connect(NULL, NULL);
  assert(gl->connection);

  gl->screen = xcb_setup_roots_iterator(xcb_get_setup(gl->connection)).data;
  assert(gl->screen);
}

void
gl_quit(gl_context_t *gl)
{
  assert(gl);

  xcb_disconnect(gl->connection);
  gl->connection = NULL;
  gl->screen = NULL;
}

void
gl_create_window(gl_context_t *gl, gl_window_t *window, i32_t width, i32_t height)
{
  assert(gl);
  assert(window);


  // create window

  u32_t value_mask;
  u32_t value_list[2];

  window->width = width;
  window->height = height;
  window->byte_size = width * height * 4;

  window->id = xcb_generate_id(gl->connection);

  value_mask = XCB_CW_BACK_PIXEL  // value_list[0]
             | XCB_CW_EVENT_MASK; // value_list[1]

  value_list[0] = gl->screen->black_pixel;  // XCB_CW_BACK_PIXEL

  value_list[1] = XCB_EVENT_MASK_EXPOSURE   // XCB_CW_EVENT_MASK
                | XCB_EVENT_MASK_KEY_PRESS
                | XCB_EVENT_MASK_STRUCTURE_NOTIFY; // -> WM_DELETE_WINDOW

  xcb_create_window(
    gl->connection,
    XCB_COPY_FROM_PARENT,
    window->id,
    gl->screen->root,
    0, 0, window->width, window->height, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    gl->screen->root_visual,
    value_mask, value_list
  );


  // create frame_buffer / image

  window->frame_buffer = malloc(window->byte_size);
  assert(window->frame_buffer);

  window->image = xcb_image_create_native(
    gl->connection,
    window->width, window->height,
    XCB_IMAGE_FORMAT_Z_PIXMAP,
    gl->screen->root_depth,
    NULL, 0, (u8_t *) window->frame_buffer
  );
  assert(window->image);


  // create pixmap

  gl->pixmap = xcb_generate_id(gl->connection);

  xcb_create_pixmap(
    gl->connection,
    gl->screen->root_depth,
    gl->pixmap,
    window->id,
    window->width, window->height
  );


  // create gc

  window->gc = xcb_generate_id(gl->connection);
  xcb_create_gc(gl->connection, window->gc, window->id, 0, NULL);


  // set WM_DELETE_WINDOW

  const char property_name[] = "WM_PROTOCOLS";
  const char atom_name[]     = "WM_DELETE_WINDOW";

  xcb_intern_atom_cookie_t  property_cookie;
  xcb_intern_atom_cookie_t  atom_cookie;
  xcb_intern_atom_reply_t  *property_reply;
  xcb_intern_atom_reply_t  *atom_reply;

  xcb_atom_t properties[1];

  property_cookie = xcb_intern_atom(
    gl->connection, 0, sizeof(property_name) - 1, property_name);

  atom_cookie = xcb_intern_atom(
    gl->connection, 0, sizeof(atom_name) - 1, atom_name);

  property_reply = xcb_intern_atom_reply(gl->connection, property_cookie, NULL);
  atom_reply = xcb_intern_atom_reply(gl->connection, atom_cookie, NULL);
  assert(property_reply);
  assert(atom_reply);

  properties[0] = atom_reply->atom;

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    window->id,
    property_reply->atom,
    XCB_ATOM_ATOM,
    sizeof(i32_t) * 8,
    sizeof(properties) / sizeof(properties[0]),
    properties
  );

  window->wm_delete_atom = atom_reply->atom;

  free(property_reply);
  free(atom_reply);


  // map window

  xcb_map_window(gl->connection, window->id);
}

void
gl_destroy_window(gl_window_t *window)
{
  free(window->frame_buffer);
  // @todo window->image
}

void
gl_show_window(gl_context_t *gl, gl_window_t *window)
{
  assert(gl);
  assert(window);
  xcb_map_window(gl->connection, window->id);
  xcb_flush(gl->connection); // ?
}


void
gl_set_string_property(gl_context_t *gl, gl_window_t *window, const char *atom_name, const char *atom_value)
{
  assert(gl);
  assert(window);
  assert(atom_name);
  assert(atom_value);

  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  cookie = xcb_intern_atom(gl->connection, 0, sizeof(atom_name) - 1, atom_name);
  reply  = xcb_intern_atom_reply(gl->connection, cookie, NULL);
  assert(reply);

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    window->id,
    reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    sizeof(atom_value) - 1,
    atom_value
  );

  free(reply);
}


int
main(void)
{
  gl_context_t gl;
  gl_window_t  win;

  gl_init(&gl);

  gl_create_window(&gl, &win, 990, 540);
  gl_set_string_property(&gl, &win, "WM_CLASS", "xcbexample\0XCBExample\0");
  gl_set_string_property(&gl, &win, "WM_NAME", "XCB Example");

  gl.loop = true;

  for (gl.loop = true; gl.loop; )
  {
    while ((gl.event = xcb_poll_for_event(gl.connection)))
    {
      switch (gl.event->response_type & ~0x80)
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) gl.event;
          if (key->detail == KEY_ESC) gl.loop = false;
          break;
      }

      free(gl.event);
    }
  }

  gl_destroy_window(&win);
  gl_quit(&gl);
}


/*
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
*/
