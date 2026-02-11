
// file: src/gl/gl.h

#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include "../core/core.h"


// Key codes

// @todo linux wayland
// @todo android
// @todo ios
// @todo error handling

#if defined(__linux__) || defined(__unix__) || defined(__gnu_linux__)

  #define KEY_ESC 9  // Linux/Unix X11

#elif defined(__APPLE__)

  #define KEY_ESC 41 // MacOS HID

#elif defined(_WIN32) || defined(_WIN64)

  #define KEY_ESC 27 // Windows VK_ESCAPE or ASCII (0x1B)

#endif


// Data structures

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
  i32_t           size;
  i32_t           byte_size;
  double          aspect_ratio;
  i32_t          *frame_buffer;
  xcb_image_t    *image;
  xcb_atom_t      wm_delete_atom;
} gl_window_t;

typedef u32_t color_t;


// API methods

void   gl_init(gl_context_t *gl);
void   gl_quit(gl_context_t *gl);

void   gl_create_window(gl_context_t *gl, gl_window_t *window, i32_t width, i32_t height);
void   gl_destroy_window(gl_context_t *gl, gl_window_t *window);
void   gl_show_window(gl_context_t *gl, gl_window_t *window);
void   gl_blit_window(gl_context_t *gl, gl_window_t *window);

void   gl_set_string_property(gl_context_t *gl, gl_window_t *window, const char *atom_name, const char *atom_value, usize_t value_size);

void   gl_start(gl_context_t *gl);
void   gl_stop(gl_context_t *gl);
bool_t gl_is_running(gl_context_t *gl);

void   gl_fill_window(gl_window_t *window, color_t color);
void   gl_put_pixel(gl_window_t *window, i32_t x, i32_t y, color_t color);
void   gl_draw_line(gl_window_t *window, i32_t x0, i32_t y0, i32_t x1, i32_t y1, color_t color);

i32_t  gl_project_x(gl_window_t *window, double x);
i32_t  gl_project_y(gl_window_t *window, double y);


#ifdef GL_IMPLEMENTATION

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
}

void
gl_create_window(gl_context_t *gl, gl_window_t *window, i32_t width, i32_t height)
{
  assert(gl);
  assert(window);


  // create window

  u32_t value_mask;
  u32_t value_list[2];

  window->width  = width;
  window->height = height;

  window->size         = window->width * window->height;
  window->byte_size    = window->size  * sizeof(i32_t);
  window->aspect_ratio = (double) window->width / (double) window->height;

  window->id = xcb_generate_id(gl->connection);

  value_mask    = XCB_CW_BACK_PIXEL         // value_list[0]
                | XCB_CW_EVENT_MASK;        // value_list[1]

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

  window->frame_buffer = malloc(window->byte_size * sizeof(i32_t));
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
}

void
gl_destroy_window(gl_context_t *gl, gl_window_t *window)
{
  assert(gl);
  assert(window);
  xcb_free_gc(gl->connection, window->gc);     // ?
  xcb_free_pixmap(gl->connection, gl->pixmap); // ?
  xcb_image_destroy(window->image);
  free(window->frame_buffer);
}

void
gl_show_window(gl_context_t *gl, gl_window_t *window)
{
  assert(gl);
  assert(window);
  xcb_map_window(gl->connection, window->id);
  xcb_flush(gl->connection);
}

void
gl_blit_window(gl_context_t *gl, gl_window_t *window)
{
  assert(gl);
  assert(window);

  xcb_image_put(
    gl->connection,
    gl->pixmap,
    window->gc,
    window->image,
    0, 0, 0 // x, y, left_pad
  );

  xcb_copy_area(
    gl->connection,
    gl->pixmap,
    window->id,
    window->gc,
    0, 0, 0, 0,
    window->width, window->height
  );

  xcb_flush(gl->connection);
}


void
gl_set_string_property(
  gl_context_t *gl,
  gl_window_t  *window,
  const char   *atom_name,
  const char   *atom_value,
  usize_t       value_size
) {
  assert(gl);
  assert(window);
  assert(atom_name);
  assert(atom_value);

  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  cookie = xcb_intern_atom(gl->connection, 0, string_length(atom_name), atom_name);
  reply  = xcb_intern_atom_reply(gl->connection, cookie, NULL);
  assert(reply);

  if (value_size == 0) value_size = string_length(atom_value);

  xcb_change_property(
    gl->connection,        // connection
    XCB_PROP_MODE_REPLACE, // mode
    window->id,            // window
    reply->atom,           // property
    XCB_ATOM_STRING,       // type
    sizeof(char) * 8,      // format
    value_size,            // data_len
    atom_value             // data
  );

  free(reply);
}

void
gl_start(gl_context_t *gl)
{
  assert(gl);
  gl->loop = true;
}

void
gl_stop(gl_context_t *gl)
{
  assert(gl);
  gl->loop = false;
}

bool_t
gl_is_running(gl_context_t *gl)
{
  assert(gl);
  return gl->loop;
}


void
gl_fill_window(gl_window_t *window, color_t color)
{
  assert(window);

  for (i32_t i = 0; i < window->size; ++i)
    window->frame_buffer[i] = color;
}

void
gl_put_pixel(gl_window_t *window, i32_t x, i32_t y, color_t color)
{
  assert(window);

  if (
       x >= 0 && x < window->width
    && y >= 0 && y < window->height
  ) {
    window->frame_buffer[y * window->width + x] = color;
  }
}

void
gl_draw_line(gl_window_t *window, i32_t x0, i32_t y0, i32_t x1, i32_t y1, color_t color)
{
  assert(window);

  i32_t dx  =  absolute_number(x1 - x0);
  i32_t dy  = -absolute_number(y1 - y0);
  i32_t sx  =  x0 < x1 ? 1 : -1;
  i32_t sy  =  y0 < y1 ? 1 : -1;
  i32_t err =  dx + dy;

  while (true)
  {
    gl_put_pixel(window, x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    i32_t err2 = err * 2;
    if (err2 >= dy) { err += dy; x0 += sx; }
    if (err2 <= dx) { err += dx; y0 += sy; }
  }
}


i32_t
gl_project_x(gl_window_t *window, double x)
{
  assert(window);
  return (i32_t) (((double) window->width * .5) * (1.0 + x / window->aspect_ratio));
}

i32_t
gl_project_y(gl_window_t *window, double y)
{
  assert(window);
  return (i32_t) (((double) window->height * .5) * (1.0 + y));
}

#endif
