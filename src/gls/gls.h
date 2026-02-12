
// file: src/gls/gls.h

#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include "../core/core.h"


// Key codes

// @todo add wayland support
// @todo add android support
// @todo add ios     support

#if defined(__linux__) || defined(__unix__) || defined(__gnu_linux__)

  #define KEY_ESC 9  // Linux/Unix X11

#elif defined(__APPLE__)

  #define KEY_ESC 41 // MacOS HID

#elif defined(_WIN32) || defined(_WIN64)

  #define KEY_ESC 27 // Windows VK_ESCAPE or ASCII (0x1B)

#else

  #error "Unsuported platform"

#endif


// Data structures

typedef struct gls_context_s gls_context_t;
typedef struct gls_window_s  gls_window_t;
typedef u32_t  argb_color_t;


// API methods

void   gls_init(gls_context_t *ctx);
void   gls_quit(gls_context_t *ctx);

void   gls_create_window(gls_context_t *ctx, gls_window_t *window, i32_t width, i32_t height);
void   gls_destroy_window(gls_context_t *ctx, gls_window_t *window);
void   gls_show_window(gls_context_t *ctx, gls_window_t *window);
void   gls_blit_window(gls_context_t *ctx, gls_window_t *window);

void   gls_set_string_property(gls_context_t *ctx, gls_window_t *window, const char *atom_name, const char *atom_value, usize_t value_size);

void   gls_start(gls_context_t *ctx);
void   gls_stop(gls_context_t *ctx);
bool_t gls_is_running(gls_context_t *ctx);

void   gls_fill_window(gls_window_t *window, argb_color_t color);
void   gls_put_pixel(gls_window_t *window, i32_t x, i32_t y, argb_color_t color);
void   gls_draw_line(gls_window_t *window, i32_t x0, i32_t y0, i32_t x1, i32_t y1, argb_color_t color);

i32_t  gls_project_x(gls_window_t *window, double x);
i32_t  gls_project_y(gls_window_t *window, double y);


#ifdef GLS_IMPLEMENTATION


typedef struct gls_context_s
{
  xcb_connection_t     *connection;
  xcb_screen_t         *screen;
  xcb_generic_event_t  *event;
  bool_t                loop;
} gls_context_t;

typedef struct gls_window_s
{
  xcb_window_t    id;
  xcb_gcontext_t  gc;
  i32_t           width, height; // ? u32_t
  i32_t           size;          // ? u32_t
  i32_t           byte_size;     // ? u32_t
  double          aspect_ratio;
  i32_t          *frame_buffer;  // ? u32_t
  xcb_image_t    *image;
  xcb_pixmap_t    pixmap;
  xcb_atom_t      wm_delete_atom;
} gls_window_t;


void
gls_init(gls_context_t *ctx)
{
  assert(ctx);

  ctx->connection = xcb_connect(NULL, NULL);
  assert(ctx->connection);

  ctx->screen = xcb_setup_roots_iterator(xcb_get_setup(ctx->connection)).data;
  assert(ctx->screen);
}

void
gls_quit(gls_context_t *ctx)
{
  assert(ctx);
  xcb_disconnect(ctx->connection);
}

void
gls_create_window(gls_context_t *ctx, gls_window_t *window, i32_t width, i32_t height)
{
  assert(ctx);
  assert(window);


  // create window

  u32_t value_mask;
  u32_t value_list[2];

  window->width  = width;
  window->height = height;

  window->size         = window->width * window->height;
  window->byte_size    = window->size  * sizeof(i32_t);
  window->aspect_ratio = (double) window->width / (double) window->height;

  window->id    = xcb_generate_id(ctx->connection);

  value_mask    = XCB_CW_BACK_PIXEL         // value_list[0]
                | XCB_CW_EVENT_MASK;        // value_list[1]

  value_list[0] = ctx->screen->black_pixel;  // XCB_CW_BACK_PIXEL

  value_list[1] = XCB_EVENT_MASK_EXPOSURE   // XCB_CW_EVENT_MASK
                | XCB_EVENT_MASK_KEY_PRESS
                | XCB_EVENT_MASK_STRUCTURE_NOTIFY; // -> WM_DELETE_WINDOW

  xcb_create_window(
    ctx->connection,
    XCB_COPY_FROM_PARENT,
    window->id,
    ctx->screen->root,
    0, 0, window->width, window->height, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    ctx->screen->root_visual,
    value_mask, value_list
  );


  // create frame_buffer / image

  window->frame_buffer = malloc(window->byte_size * sizeof(i32_t));
  assert(window->frame_buffer);

  window->image = xcb_image_create_native(
    ctx->connection,
    window->width, window->height,
    XCB_IMAGE_FORMAT_Z_PIXMAP,
    ctx->screen->root_depth,
    NULL, 0, (u8_t *) window->frame_buffer
  );
  assert(window->image);


  // create pixmap

  window->pixmap = xcb_generate_id(ctx->connection);

  xcb_create_pixmap(
    ctx->connection,
    ctx->screen->root_depth,
    window->pixmap,
    window->id,
    window->width, window->height
  );


  // create gc

  window->gc = xcb_generate_id(ctx->connection);
  xcb_create_gc(ctx->connection, window->gc, window->id, 0, NULL);


  // set WM_DELETE_WINDOW

  const char property_name[] = "WM_PROTOCOLS";
  const char atom_name[]     = "WM_DELETE_WINDOW";

  xcb_intern_atom_cookie_t  property_cookie;
  xcb_intern_atom_cookie_t  atom_cookie;
  xcb_intern_atom_reply_t  *property_reply;
  xcb_intern_atom_reply_t  *atom_reply;

  xcb_atom_t properties[1];

  property_cookie = xcb_intern_atom(
    ctx->connection, 0, sizeof(property_name) - 1, property_name);

  atom_cookie = xcb_intern_atom(
    ctx->connection, 0, sizeof(atom_name) - 1, atom_name);

  property_reply = xcb_intern_atom_reply(ctx->connection, property_cookie, NULL);
  atom_reply = xcb_intern_atom_reply(ctx->connection, atom_cookie, NULL);
  assert(property_reply);
  assert(atom_reply);

  properties[0] = atom_reply->atom;

  xcb_change_property(
    ctx->connection,
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
gls_destroy_window(gls_context_t *ctx, gls_window_t *window)
{
  assert(ctx);
  assert(window);
  xcb_free_gc(ctx->connection, window->gc);
  xcb_free_pixmap(ctx->connection, window->pixmap);
  xcb_image_destroy(window->image);
  free(window->frame_buffer);
}

void
gls_show_window(gls_context_t *ctx, gls_window_t *window)
{
  assert(ctx);
  assert(window);
  xcb_map_window(ctx->connection, window->id);
  xcb_flush(ctx->connection);
}

void
gls_blit_window(gls_context_t *ctx, gls_window_t *window)
{
  assert(ctx);
  assert(window);

  xcb_image_put(
    ctx->connection,
    window->pixmap,
    window->gc,
    window->image,
    0, 0, 0 // x, y, left_pad
  );

  xcb_copy_area(
    ctx->connection,
    window->pixmap,
    window->id,
    window->gc,
    0, 0, 0, 0,
    window->width, window->height
  );

  xcb_flush(ctx->connection);
}


void
gls_set_string_property(
  gls_context_t *ctx,
  gls_window_t  *window,
  const char    *atom_name,
  const char    *atom_value,
  usize_t        value_size
) {
  assert(ctx);
  assert(window);
  assert(atom_name);
  assert(atom_value);

  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  cookie = xcb_intern_atom(ctx->connection, 0, string_length(atom_name), atom_name);
  reply  = xcb_intern_atom_reply(ctx->connection, cookie, NULL);
  assert(reply);

  if (value_size == 0) value_size = string_length(atom_value);

  xcb_change_property(
    ctx->connection,       // connection
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
gls_start(gls_context_t *ctx)
{
  assert(ctx);
  ctx->loop = true;
}

void
gls_stop(gls_context_t *ctx)
{
  assert(ctx);
  ctx->loop = false;
}

bool_t
gls_is_running(gls_context_t *ctx)
{
  assert(ctx);
  return ctx->loop;
}


void
gls_fill_window(gls_window_t *window, argb_color_t color)
{
  assert(window);

  for (i32_t i = 0; i < window->size; ++i)
    window->frame_buffer[i] = color;
}

void
gls_put_pixel(gls_window_t *window, i32_t x, i32_t y, argb_color_t color)
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
gls_draw_line(gls_window_t *window, i32_t x0, i32_t y0, i32_t x1, i32_t y1, argb_color_t color)
{
  assert(window);

  i32_t dx  =  absolute_number(x1 - x0);
  i32_t dy  = -absolute_number(y1 - y0);
  i32_t sx  =  x0 < x1 ? 1 : -1;
  i32_t sy  =  y0 < y1 ? 1 : -1;
  i32_t err =  dx + dy;

  while (true)
  {
    gls_put_pixel(window, x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    i32_t err2 = err * 2;
    if (err2 >= dy) { err += dy; x0 += sx; }
    if (err2 <= dx) { err += dx; y0 += sy; }
  }
}


i32_t
gls_project_x(gls_window_t *window, double x)
{
  assert(window);
  return (i32_t) (((double) window->width * .5) * (1.0 + x / window->aspect_ratio));
}

i32_t
gls_project_y(gls_window_t *window, double y)
{
  assert(window);
  return (i32_t) (((double) window->height * .5) * (1.0 + y));
}

#endif
