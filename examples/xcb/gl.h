
// file: xcb/gl.h

#pragma once

#include <xcb/xcb.h>
#include "core/core.h"


// KEY codes

#ifdef __APPLE__
  #define KEY_ESC 41 // MacOS HID
#else
  #define KEY_ESC 9  // Linux X11
#endif


// gl structure (Graphic Library)

typedef struct gl_s gl_t;
typedef void (*gl_event_handler_t)(gl_t *gl);

typedef struct gl_s
{
  // game loop switcher
  bool_t loop;

  // X connection entities
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_generic_event_t *event;

  // default font
  xcb_font_t font;
  xcb_gcontext_t gcontext;

  // handlers
  gl_event_handler_t expose_handler;
  gl_event_handler_t key_press_handler;
} gl_t;

bool_t gl_create_window(gl_t *gl, u16_t width, u16_t height, const char *fontname);
void   gl_destroy_window(gl_t *gl);

void   gl_open_font(gl_t *gl, const char *fontname);
void   gl_close_font(gl_t *gl);

void   gl_loop_frames(gl_t *gl);
