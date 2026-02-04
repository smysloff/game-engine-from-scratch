
// file: examples/xcb/bit_blit.c

#include <xcb/xcb.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct
{
  xcb_connection_t *connection;
  xcb_screen_t     *screen;
  xcb_window_t      window;
} gl_state_t;

typedef struct
{
  gl_state_t state;
} gl_t;

bool gl_init(void);
bool gl_create_window(uint32_t width, uint32_t height);
void gl_change_property(const char *atom, const char *value, size_t len);
void gl_poll_event(void);
void gl_quit(void);

static gl_t gl;


int
main(void)
{
  gl_init();
  gl_create_window(768, 432);
  gl_poll_event();
  gl_quit();
}


bool
gl_init(void)
{
  int                    screen_number;
  const xcb_setup_t     *setup;
  xcb_screen_iterator_t  screen_iterator;


  // Set connection to X-Server

  gl.state.connection = xcb_connect(NULL, &screen_number);

  if (xcb_connection_has_error(gl.state.connection))
    return false;


  // Get screen

  setup = xcb_get_setup(gl.state.connection);
  screen_iterator = xcb_setup_roots_iterator(setup);
  //gl.screen = NULL;

  while (screen_iterator.rem)
  {
    if (screen_number == 0)
    {
      gl.state.screen = screen_iterator.data;
      break;
    }
    xcb_screen_next(&screen_iterator);
    --screen_number;
  }

  if (!gl.state.screen)
    return false;


  return true;
}

bool
gl_create_window(uint32_t width, uint32_t height)
{
  uint32_t cw_value_mask;
  uint32_t cw_value_list[2];


  cw_value_mask = XCB_CW_BACK_PIXEL
                | XCB_CW_EVENT_MASK;

  cw_value_list[0] = gl.state.screen->black_pixel;

  cw_value_list[1] = XCB_EVENT_MASK_EXPOSURE
                   | XCB_EVENT_MASK_KEY_PRESS;

  gl.state.window = xcb_generate_id(gl.state.connection);

  xcb_create_window(
    gl.state.connection,
    XCB_COPY_FROM_PARENT,
    gl.state.window,
    gl.state.screen->root,
    0, 0, width, height, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    gl.state.screen->root_visual,
    cw_value_mask, cw_value_list
  );


  // Set size hints

  const char hints_atom_name[] = "WM_NORMAL_HINTS";
  int32_t hints[13] = { 0 };

  xcb_intern_atom_cookie_t  hints_cookie;
  xcb_intern_atom_reply_t  *hints_reply;

  hints_cookie = xcb_intern_atom(
    gl.state.connection,
    0,
    sizeof(hints_atom_name) - 1,
    hints_atom_name
  );

  hints_reply = xcb_intern_atom_reply(
    gl.state.connection,
    hints_cookie,
    NULL
  );

  if (hints_reply)
  {
    hints[0] = (1 << 4) | (1 << 5);
    hints[5] = width;
    hints[6] = height;
    hints[7] = width;
    hints[8] = height;

    xcb_change_property(
      gl.state.connection,
      XCB_PROP_MODE_REPLACE,
      gl.state.window,
      hints_reply->atom,
      hints_reply->atom,
      sizeof(int32_t) * 8,
      sizeof(hints) / sizeof(hints[0]),
      hints
    );

    free(hints_reply);
    hints_reply = NULL;
  }


  // Set WM_CLASS

  const char wm_class_atom[] = "WM_CLASS";
  const char wm_class_value[] = "xcbexample\0XCBExample\0";
  gl_change_property(wm_class_atom, wm_class_value, sizeof(wm_class_value) - 1);


  // Set WM_NAME (TITLE)

  const char wm_name_atom[] = "WM_NAME";
  const char wm_name_value[] = "XCB Example";
  gl_change_property(wm_name_atom, wm_name_value, sizeof(wm_name_value) - 1);


  // Show window

  xcb_map_window(gl.state.connection, gl.state.window);
  xcb_flush(gl.state.connection);


  // Exit

  return true;
}

void
gl_change_property(const char *atom, const char *value, size_t len)
{
  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  cookie = xcb_intern_atom(gl.state.connection, 0, sizeof(atom), atom);
  reply = xcb_intern_atom_reply(gl.state.connection, cookie, NULL);

  if (reply)
  {
    xcb_change_property(
      gl.state.connection,
      XCB_PROP_MODE_REPLACE,
      gl.state.window,
      reply->atom,
      XCB_ATOM_STRING,
      sizeof(char) * 8,
      len,
      value
    );

    free(reply);
    reply = NULL;
  }
}

void
gl_poll_event(void)
{
  printf("gl_poll_event\n");
  sleep(3);
}

void
gl_quit(void)
{
  xcb_disconnect(gl.state.connection);
  gl.state.connection = NULL;
  gl.state.screen = NULL;
}
