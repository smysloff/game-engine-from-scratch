
// file: examples/fonts.c

#include <xcb/xcb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "core/core.h"


// Key codes

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
  xcb_gcontext_t gcontext;
  xcb_font_t font;
  xcb_generic_event_t *event;
  bool_t loop;
} gl_t;


void gl_create_window(gl_t *gl, u16_t width, u16_t height);
void gl_destroy_window(gl_t *gl);

void gl_open_font(gl_t *gl, const char *fontname);
void gl_close_font(gl_t *gl);

int
main(void)
{
  gl_t gl;

  const char fontname[] = "-xos4-terminus-medium-r-normal--32-320-72-72-c-160-iso10646-1";
  const char msg[] = "Hello, World!";

  gl_create_window(&gl, 640, 480);
  gl_open_font(&gl, fontname);


  // game loop

  printf("press ESC to exit\n");

  for (gl.loop = true; gl.loop; )
  {
    while ((gl.event = xcb_poll_for_event(gl.connection)))
    {
      switch (gl.event->response_type & ~0x80)
      {
        case XCB_EXPOSE:
        {
          xcb_image_text_8(
            gl.connection,
            strlen(msg),
            gl.window,
            gl.gcontext,
            64, 64, msg
          );
          xcb_flush(gl.connection);
          break;
        };

        case XCB_KEY_PRESS:
        {
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) gl.event;
          printf("key press: %d\n", key->detail);
          if (key->detail == KEY_ESC)
            gl.loop = false;
          break;
        }

      } // switch
      free(gl.event);
    } // while
    usleep(10000);
  }

  gl_close_font(&gl);
  gl_destroy_window(&gl);
}

void
gl_create_window(gl_t *gl, u16_t width, u16_t height)
{
  uint32_t cw_mask;
  uint32_t cw_list[2];

  gl->connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(gl->connection))
  {
    print_error("Cannot connect to X server");
    exit(-1); // @todo
  }

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
gl_open_font(gl_t *gl, const char *fontname)
{
  xcb_void_cookie_t cookie;
  xcb_generic_error_t *error;

  uint32_t cw_mask;
  uint32_t cw_list[3];

  const char default_font[] = "fixed";


  gl->font = xcb_generate_id(gl->connection);

  cookie = xcb_open_font_checked(
    gl->connection, gl->font, strlen(fontname), fontname);

  error = xcb_request_check(gl->connection, cookie);

  if (error)
  {
    print_error("Font not found! Trying 'fixed' instead.");
    free(error);
    xcb_open_font(
      gl->connection,
      gl->font,
      strlen(default_font), default_font
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
  xcb_free_gc(gl->connection, gl->gcontext);
  xcb_close_font(gl->connection, gl->font);
}
