
// file: examples/xcb/fonts.c

#include <xcb/xcb.h>
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
  xcb_generic_event_t *event;
  xcb_gcontext_t gcontext;
  bool_t loop;
} gl_t;

void gl_create_window(gl_t *gl, u16_t width, u16_t height);

void gl_destroy_window(gl_t *gl);

void gl_free_event(gl_t *gl);

void gl_loop_frames(
  gl_t *gl,
  void (*render_frame)(gl_t *),
  void (*update_input)(gl_t *)
);


void render_frame(gl_t *gl);

void update_input(gl_t *gl);


int
main(void)
{
  gl_t gl;

  gl_create_window(&gl, 640, 480);
  gl_loop_frames(&gl, render_frame, update_input);
  gl_destroy_window(&gl);
}


// gl methods

void
gl_loop_frames(
  gl_t *gl,
  void (*render_frame)(gl_t *),
  void (*update_input)(gl_t *)
) {
  render_frame(gl);
  print_log("press ESC key to exit");

  for (gl->loop = true; gl->loop; )
  {
    while ((gl->event = xcb_poll_for_event(gl->connection)))
    {
      switch (gl->event->response_type & ~0x80)
      {
        case XCB_EXPOSE:    render_frame(gl); break;
        case XCB_KEY_PRESS: update_input(gl); break;
      }
    }
  }
}

void
gl_create_window(gl_t *gl, u16_t width, u16_t height)
{
  u32_t value_mask;
  u32_t value_list[2];

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


// event handlers

void
render_frame(gl_t *gl)
{
  xcb_expose_event_t *e = (xcb_expose_event_t *) gl->event;

  (void) e;
}

void
update_input(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_string("log: pressed key with code: ");
  print_number(e->detail);
  print_endl();

  if (e->detail == KEY_ESC)
    gl->loop = false;
}
