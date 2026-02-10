
// file: examples/xcb/bit_blit.c


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>


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
  bool                  loop;
  uint64_t              frame;
} gl_context_t;

typedef struct
{
  xcb_window_t  id;
  uint32_t      width, height;
  uint32_t      byte_size;
  uint8_t      *buffer;
  xcb_image_t  *image;
  xcb_atom_t    wm_delete_atom;
} gl_window_t;

typedef union
{
  uint32_t value;
  struct
  {
    uint8_t b; // blue
    uint8_t g; // green
    uint8_t r; // red
    uint8_t a; // alpha
  } components;
} gl_color_t;


// Graphic Library API

void gl_init(gl_context_t *gl);
void gl_quit(gl_context_t *gl);
void gl_start(gl_context_t *gl);
void gl_stop(gl_context_t *gl);
bool gl_is_running(gl_context_t *gl);

void gl_window_create(gl_context_t *gl, gl_window_t *window, uint32_t width, uint32_t height);
void gl_window_destroy(gl_window_t *window);
void gl_window_set_class_name(gl_context_t *gl, gl_window_t *window, const char *class_name);
void gl_winodw_set_title(gl_context_t *gl, gl_window_t *window, const char *title);
void gl_window_show(gl_context_t *gl, gl_window_t *window);

void gl_put_pixel(int32_t x, int32_t y, gl_color_t color, gl_window_t *window);


// gl_context_t

void
gl_init(gl_context_t *gl)
{
  // @todo error handling
  gl->connection = xcb_connect(NULL, NULL);
  gl->screen = xcb_setup_roots_iterator(xcb_get_setup(gl->connection)).data;
}

void
gl_quit(gl_context_t *gl)
{
  // @todo handle gl == NULL
  xcb_disconnect(gl->connection);
  gl->connection = NULL;
  gl->screen     = NULL;
}

void
gl_start(gl_context_t *gl)
{
  gl->loop = true;
  gl->frame = 0;
}

void
gl_stop(gl_context_t *gl)
{
  gl->loop = false;
}

bool
gl_is_running(gl_context_t *gl)
{
  return gl->loop;
}


// gl_window_t

void
gl_window_create(gl_context_t *gl, gl_window_t *window, uint32_t width, uint32_t height)
{
  // @todo error hanling

  uint32_t mask;
  uint32_t list[2];

  window->width     = width;
  window->height    = height;
  window->byte_size = width * height * 4;

  window->buffer = malloc(window->byte_size); // 32bpp (bits per pixel)

  window->image = xcb_image_create_native(
    gl->connection,
    window->width, window->height,
    XCB_IMAGE_FORMAT_Z_PIXMAP,
    gl->screen->root_depth,
    window->buffer, window->byte_size,
    window->buffer
  );

  window->id = xcb_generate_id(gl->connection);

  mask    = XCB_CW_BACK_PIXEL  // list[0]
          | XCB_CW_EVENT_MASK; // list[1]

  list[0] = gl->screen->black_pixel;

  list[1] = XCB_EVENT_MASK_EXPOSURE
          | XCB_EVENT_MASK_KEY_PRESS
          | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  xcb_create_window(
    gl->connection,
    XCB_COPY_FROM_PARENT,
    window->id,
    gl->screen->root,
    0, 0, window->width, window->height, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    gl->screen->root_visual,
    mask, list
  );


  // set WM_PROTOCOLS and WM_DELETE_WINDOW

  const char protocols_name[] = "WM_PROTOCOLS";
  const char delete_name[]    = "WM_DELETE_WINDOW";

  xcb_intern_atom_cookie_t  protocols_cookie;
  xcb_intern_atom_cookie_t  delete_cookie;
  xcb_intern_atom_reply_t  *protocols_reply;
  xcb_intern_atom_reply_t  *delete_reply;

  xcb_atom_t protocols[1];

  protocols_cookie = xcb_intern_atom(
    gl->connection, 0, sizeof(protocols_name) - 1, protocols_name);

  delete_cookie = xcb_intern_atom(
    gl->connection, 0, sizeof(delete_name) - 1, delete_name);

  protocols_reply = xcb_intern_atom_reply(
    gl->connection, protocols_cookie, NULL);

  delete_reply = xcb_intern_atom_reply(
    gl->connection, delete_cookie, NULL);

  // @todo ^error handling

  protocols[0] = delete_reply->atom;

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    window->id,
    protocols_reply->atom,
    XCB_ATOM_ATOM,
    sizeof(int32_t) * 8,
    sizeof(protocols) / sizeof(protocols[0]),
    protocols
  );

  window->wm_delete_atom = delete_reply->atom;

  free(protocols_reply); protocols_reply = NULL;
  free(delete_reply);    delete_reply    = NULL;
}

void
gl_window_destroy(gl_window_t *window)
{
  // @todo error handling
  free(window->buffer);
  // @todo window->image
  window->buffer         = NULL;
  window->image          = NULL;
  window->id             = 0;
  window->width          = 0;
  window->height         = 0;
  window->byte_size      = 0;
  window->wm_delete_atom = 0;
}

void
gl_window_show(gl_context_t &gl, gl_window_t *window)
{
  // @todo error handling
  xcb_map_window(gl->connection, window->id);
  xcb_flush(gl->connection);
}

void
gl_window_set_class_name(gl_context_t *gl, gl_window_t *window, const char *class_name)
{
  // @todo error handling

  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  const char atom_name[] = "WM_CLASS";

  cookie = xcb_intern_atom(gl->connection, 0, sizeof(atom_name) - 1, atom_name);
  reply  = xcb_intern_atom_reply(gl->connection, cookie, NULL);

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    window->id,
    reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    sizeof(class_name) - 1,
    class_name
  );

  free(reply); reply = NULL;
}

void
gl_window_set_title(gl_context_t *gl, gl_window_t *window, const char *title)
{
  // @todo error handling

  xcb_intern_atom_cookie_t  cookie;
  xcb_intern_atom_reply_t  *reply;

  const char atom_name[] = "WM_NAME";

  cookie = xcb_intern_atom(gl->connection, 0, sizeof(atom_name) - 1, atom_name);
  reply  = xcb_intern_atom_reply(gl->connection, cookie, NULL);

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    window->id,
    reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    sizeof(title) - 1,
    title
  );

  free(reply); reply = NULL;
}


// gl_color_t

void
gl_put_pixel(int32_t x, int32_t y, gl_color_t color, gl_window_t *window)
{

  // check boundaries

  if (
       x < 0 || x >= window->width
    || y < 0 || y >= window->height
  ) {
    return;
  }


  // per byte copying

  //int32_t offset = (y * window->width + x) * 4;
  //window->buffer[offset + 0] = color.components.b;
  //window->buffer[offset + 1] = color.components.g;
  //window->buffer[offset + 2] = color.components.r;
  //window->buffer[offset + 3] = color.components.a;


  // 4-byte copying
  // @warning: works only if little-endians

  uint32_t offset = (y * window->width + x) * 4;
  uint32_t *pixel_ptr = (uint32_t *) &window->buffer[offset];

  *pixel_ptr = color.value;
}


int
main(void)
{
  gl_context_t  gl;
  gl_window_t   window;

  gl_init(&gl);

  gl_window_create(&gl, &window, 990, 540);
  gl_window_set_class_name(&gl, &window, "xcbexample\0XCBExample\0");
  gl_window_set_title(&gl, &window, "XCB Example");


  // entity

  int32_t pos_x  = 0;
  int32_t pos_y  = 32;
  int32_t width  = 32;
  int32_t height = 32;
  int32_t speed  = 2;

  gl_color_t color = 0xffffff << 8; // RGB << A


  // create graphic context

  xcb_gcontext_t gc = xcb_generate_id(gl.connection);
  xcb_create_gc(gl.connection, gc, window.id, 0, NULL);


  // show window

  gl_window_show(&gl, &window);


  // game loop

  gl_start(&gl);

  while (gl_is_running(&gl))
  {
    while ((gl.event = xcb_poll_for_event(gl.connection))) // @todo add method
    {
      switch (gl.event->response_type & ~0x80) // @todo add method
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) gl.event; // @todo add method
          if (key->detail == KEY_ESC) gl_stop(&gl);
          break;
      }
      free(gl.event); gl.event = NULL; // @todo add method
    }

    // simple animation

    pos_x += speed;

    if (pos_x < 0) // left border
    {
      pos_x  =  0;
      speed  = -speed;
      pos_x +=  speed;
    }

    if (pos_x + width >= window.width) // right border
    {
      pos_x  =  window_width - 1 - width;
      speed  = -speed;
      pos_x +=  speed;
    }

    // clean image

    memset(window->buffer, 0, window->byte_size);

    // draw rect

    int32_t max_y = pos_y + height;
    int32_t max_x = pos_x + width;

    for (int32_t y = pos_y; y < max_y; ++y)
    {
      for (int32_t x = pos_x; x < max_x; ++x)
      {
        gl_put_pixel(x, y, color, &window);
        //int32_t offset = (y * SCREEN_WIDTH + x) * 4;
        //data[offset + 0] = 255; // blue
        //data[offset + 1] = 255; // green
        //data[offset + 2] = 255; // red
        //data[offset + 3] = 0;   // alpha
      }
    }


    // bit blit

    xcb_image_put(gl.connection, window.id, gc, window.image, 0, 0, 0);
    xcb_flush(gl.connection);


    // update frame

    ++gl.frame; // @todo add method
    usleep(16666); // ~60 FPS
  }


  // close connection and cleanup

  xcb_image_destroy(window.image); // this also frees data
  gl_window_destroy(&window);
  gl_quit(&gl);


  // exit program

  return 0;
}
