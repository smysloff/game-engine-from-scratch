
// file: examples/xcb/bit_blit.c

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

const int32_t SCREEN_WIDTH  = 990;
const int32_t SCREEN_HEIGHT = 540;
const int32_t IMAGE_BYTES   = SCREEN_WIDTH * SCREEN_HEIGHT * 4;

int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_t *screen;

  xcb_window_t window;
  uint32_t mask;
  uint32_t list[2];

  uint8_t *data;
  xcb_image_t *image;

  xcb_gcontext_t gc;

  uint32_t frame;
  bool loop;

  int32_t pos_x;
  int32_t pos_y;


  // set connection and get screen

  connection = xcb_connect(NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;


  // create window

  window = xcb_generate_id(connection);

  mask = XCB_CW_BACK_PIXEL
       | XCB_CW_EVENT_MASK;

  list[0] = screen->black_pixel;

  list[1] = XCB_EVENT_MASK_EXPOSURE
          | XCB_EVENT_MASK_KEY_PRESS;

  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,
    window,
    screen->root,
    0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    mask, list
  );


  // set WM_CLASS

  xcb_intern_atom_cookie_t cookie;
  xcb_intern_atom_reply_t *reply;

  const char atom_name[] = "WM_CLASS";
  const char atom_value[] = "xcbexample\0XCBExample\0";

  size_t atom_name_len = sizeof(atom_name) - 1;
  size_t atom_value_len = sizeof(atom_value) - 1;

  cookie = xcb_intern_atom(connection, 0, atom_name_len, atom_name);
  reply = xcb_intern_atom_reply(connection, cookie, NULL);

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    atom_value_len,
    atom_value
  );

  free(reply);
  reply = NULL;


  // show window

  xcb_map_window(connection, window);
  xcb_flush(connection);


  // create image

  data = malloc(IMAGE_BYTES); // 32bpp

  image = xcb_image_create_native(
    connection,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    XCB_IMAGE_FORMAT_Z_PIXMAP,
    screen->root_depth,
    data, IMAGE_BYTES,
    data
  );


  // create graphic context

  gc = xcb_generate_id(connection);

  xcb_create_gc(connection, gc, window, 0, NULL);


  // game loop

  frame = 0;
  loop = true;

  while (loop)
  {
    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event(connection)))
    {
      switch (event->response_type & ~0x80)
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) event;
          if (key->detail == 9)
            loop = false;
          break;
      }
    }

    // simple animation

    pos_x = (frame * 2) % SCREEN_WIDTH;
    pos_y = (frame * 3) % SCREEN_HEIGHT;

    // clean image

    memset(data, 0, IMAGE_BYTES);

    // draw rect

    for (int32_t y = pos_y; y < pos_y + 50 && y < SCREEN_HEIGHT - 50; ++y)
    {
      for (int32_t x = pos_x; x < pos_x + 50 && x < SCREEN_WIDTH - 50; ++x)
      {
        int offset = (y * SCREEN_WIDTH + x) * 4;
        data[offset + 0] = 255; // blue
        data[offset + 1] = 255; // green
        data[offset + 2] = 255; // red
        data[offset + 3] = 0;   // alpha
      }
    }


    // bit blit

    xcb_image_put(connection, window, gc, image, 0, 0, 0);
    xcb_flush(connection);


    // update frame

    ++frame;
    usleep(16666); // ~60 FPS
  }


  // close connection

  free(data);
  xcb_image_destroy(image);
  xcb_disconnect(connection);
}
