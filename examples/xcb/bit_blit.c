
// file: examples/xcb/bit_blit.c

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

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
  list[1] = XCB_EVENT_MASK_EXPOSURE;

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

  while (1)
  {

    // simple animation

    pos_x = (frame * 2) % SCREEN_WIDTH;
    pos_y = (frame * 3) % SCREEN_HEIGHT;

    // clean image

    memset(data, 0, IMAGE_BYTES);

    // draw rect

    for (int32_t y = pos_y; y < pos_y && y < SCREEN_HEIGHT; ++y)
    {
      for (int32_t x = pos_x; x < pos_x && x < SCREEN_WIDTH; ++x)
      {
        int offset = (y * SCREEN_WIDTH + x) * 4;
        data[offset + 0] = 255; // blue
        data[offset]
      }
    }

    // update frame

    ++frame;
    usleep(16666); // ~60 FPS
  }


  // close connection

  free(data);
  xcb_image_destroy(image);
  xcb_disconnect(connection);
}
