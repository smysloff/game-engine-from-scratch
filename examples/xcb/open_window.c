
// file: examples/xcb/open_window.c

//
// Include headers
//

#include <xcb/xcb.h>

#define CORE_IMPLEMENTATION
#include "core/core.h"

//
// Constants
//

#define KEY_ESC 9


//
// Function declarations
//

void game_events(xcb_connection_t *connection, bool_t *game_loop);
void game_update(void);
void game_render(void);
void game_sync_frames(u64_t delay);


//
// Main function
//

int main(void)
{
  // Variables

  xcb_connection_t *connection;
  xcb_screen_iterator_t screen_iterator;
  xcb_screen_t *screen;
  xcb_window_t window;

  bool_t game_loop = true;


  // Create connection to x11 server

  connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(connection))
  {
    print_error("xcb_connect()");
    return -1;
  }

  // Get first available screen

  const xcb_setup_t *setup = xcb_get_setup(connection);
  screen_iterator = xcb_setup_roots_iterator(setup);
  screen = screen_iterator.data;


  // Create window

  window = xcb_generate_id(connection);

  u32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  u32_t values[2] = {
    screen->black_pixel,
    XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
  };

  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,
    window,
    screen->root,
    0, 0,
    800, 600,
    10,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    mask, values
  );

  xcb_map_window(connection, window);
  xcb_flush(connection);


  // Game Loop

  print_log("press ESC to close the window");

  while (game_loop)
  {
    game_events(connection, &game_loop);
    game_update();
    game_render();
    game_sync_frames(1000 / 60);
  }


  // Disconnect from x11 server

  xcb_disconnect(connection);

  print_log("program ends successfully with code 0");

  return 0;
}


//
// Function implementaions
//

void game_events(xcb_connection_t *connection, bool_t *game_loop)
{
  xcb_generic_event_t *event;
  xcb_key_press_event_t *key_press;

  while ((event = xcb_poll_for_event(connection)))
  {
    switch (event->response_type & ~0x80)
    {
      case XCB_EXPOSE:
      {
        // @todo draw here
        break;
      }
      case XCB_KEY_PRESS:
      {
        key_press = (xcb_key_press_event_t *)event;

        print_string("log: pressed key with code: ");
        print_number(key_press->detail);
        print_endl();

        if (key_press->detail == KEY_ESC)
          *game_loop = false;

        break;
      }
    }

    free(event);
  }
}

void game_update(void)
{
  // some code...
}

void game_render(void)
{
  // some code...
}

void game_sync_frames(u64_t delay)
{
  usleep(delay);
}
