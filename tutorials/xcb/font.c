
// file: examples/xcb/fonts.c

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

void   expose_handler(gl_t *gl);
void   key_press_handler(gl_t *gl);

void   draw_frame(gl_t *gl);


int
main(void)
{
  gl_t gl;

  gl.expose_handler = expose_handler;
  gl.key_press_handler = key_press_handler;

  if (!gl_create_window(&gl, 640, 480,
        "-xos4-terminus-medium-r-normal--32-320-72-72-c-160-iso10646-1"))
  {
    print_error_endl("Cannot connect to X server");
    return -1;
  }

  gl_loop_frames(&gl);

  gl_destroy_window(&gl);

  return 0;
}


bool_t
gl_create_window(gl_t *gl, u16_t width, u16_t height, const char *fontname)
{
  uint32_t cw_mask;
  uint32_t cw_list[2];

  gl->connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(gl->connection))
    return false;

  gl->screen = xcb_setup_roots_iterator(xcb_get_setup(gl->connection)).data;
  gl->window = xcb_generate_id(gl->connection);

  cw_mask = XCB_CW_BACK_PIXEL
          | XCB_CW_EVENT_MASK;

  cw_list[0] = gl->screen->black_pixel;

  cw_list[1] = XCB_EVENT_MASK_EXPOSURE
             | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_window(
    gl->connection,
    XCB_COPY_FROM_PARENT,
    gl->window,
    gl->screen->root,
    0, 0, width, height, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    gl->screen->root_visual,
    cw_mask, cw_list
  );

  gl_open_font(gl, fontname);

  xcb_map_window(gl->connection, gl->window);
  xcb_flush(gl->connection);

  return true;
}

void
gl_destroy_window(gl_t *gl)
{
  if (gl->font)
    gl_close_font(gl);

  xcb_disconnect(gl->connection);
  gl->connection = NULL;
  gl->screen = NULL;
}

void
gl_open_font(gl_t *gl, const char *fontname)
{
  xcb_void_cookie_t cookie;
  xcb_generic_error_t *error = NULL;

  uint32_t cw_mask;
  uint32_t cw_list[3];

  const char default_font[] = "fixed";

  gl->font = xcb_generate_id(gl->connection);

  if (fontname)
  {
    cookie = xcb_open_font_checked(
      gl->connection,
      gl->font,
      string_length(fontname),
      fontname
    );

    error = xcb_request_check(gl->connection, cookie);
  }

  if (!fontname || error)
  {
    if (!fontname)
      print_error_endl("No font specified. Trying 'fixed' as default.");

    else if (error)
    {
      print_error_endl("Font not found! Trying 'fixed' instead.");
      free(error);
    }

    xcb_open_font(
      gl->connection,
      gl->font,
      string_length(default_font),
      default_font
    );
  }

  gl->gcontext = xcb_generate_id(gl->connection);

  cw_mask = XCB_GC_FOREGROUND
          | XCB_GC_BACKGROUND
          | XCB_GC_FONT;

  cw_list[0] = gl->screen->white_pixel;
  cw_list[1] = gl->screen->black_pixel;
  cw_list[2] = gl->font;

  xcb_create_gc(
    gl->connection,
    gl->gcontext,
    gl->window,
    cw_mask, cw_list
  );
}

void
gl_close_font(gl_t *gl)
{
  if (gl->font)
  {
    xcb_free_gc(gl->connection, gl->gcontext);
    xcb_close_font(gl->connection, gl->font);
  }
}

void
gl_loop_frames(gl_t *gl)
{
  draw_frame(gl);
  print_log_endl("press ESC to exit");

  for (gl->loop = true; gl->loop; )
  {

    while ((gl->event = xcb_poll_for_event(gl->connection)))
    {
      switch (gl->event->response_type & ~0x80)
      {
        case XCB_EXPOSE:
        {
          if (gl->expose_handler)
            gl->expose_handler(gl);
          break;
        }

        case XCB_KEY_PRESS:
        {
          if (gl->key_press_handler)
            gl->key_press_handler(gl);
          break;
        }
      } // switch

      free(gl->event);

    } // while

    usleep(10000);
  }

}


void
expose_handler(gl_t *gl)
{
  xcb_expose_event_t *e = (xcb_expose_event_t *) gl->event;
  (void) e;
  draw_frame(gl);
}

void
key_press_handler(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_log_endl("pressed key with code %d", e->detail);

  if (e->detail == KEY_ESC)
    gl->loop = false;
}

void
draw_frame(gl_t *gl)
{
  const char msg[] = "Hello, World!";

  xcb_image_text_8(
    gl->connection,
    string_length(msg),
    gl->window,
    gl->gcontext,
    64, 64, msg
  );

  xcb_flush(gl->connection);
}
