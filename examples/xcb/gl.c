
// file: examples/xcb/gl.c

#include "./gl.h"


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
      print_string_endl("warning: No font specified. Trying 'fixed' as default.");

    else if (error)
    {
      print_string_endl("warning: Font not found! Trying 'fixed' instead.");
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

    //usleep(10000);
  }

}
