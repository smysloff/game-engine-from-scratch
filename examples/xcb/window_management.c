
// file: examples/xcb/window_management.c

#include <xcb/xcb.h>
#include "core/core.h"


#ifdef __APPLE__
  #define KEY_ESC 41 // MacOS HID
#else
  #define KEY_ESC 9  // Linux X11
#endif


#define SCREEN_WIDTH  990
#define SCREEN_HEIGHT 540


int
main(void)
{
  xcb_connection_t *connection;
  xcb_screen_t     *screen;
  xcb_window_t      window;

  i32_t  screen_number;
  bool_t loop;



  // Set connection to X-Server

  connection = xcb_connect(NULL, &screen_number);


  if (!connection)
  {
    perror("xcb_connect()");
    return EXIT_FAILURE;
  }



  // Get screen

  const xcb_setup_t     *setup;
  xcb_screen_iterator_t  screen_iterator;


  setup = xcb_get_setup(connection);
  screen_iterator = xcb_setup_roots_iterator(setup);
  screen = NULL;


  while (screen_iterator.rem)
  {
    if (screen_number == 0)
    {
      screen = screen_iterator.data;
      break;
    }
    xcb_screen_next(&screen_iterator);
    --screen_number;
  }

  if (!screen)
  {
    print_error("Cannot get screen");
    return -1;
  }



  // Create window

  u32_t cw_value_mask;
  u32_t cw_value_list[2];


  window = xcb_generate_id(connection);

  cw_value_mask = XCB_CW_BACK_PIXEL
                | XCB_CW_EVENT_MASK;

  cw_value_list[0] = screen->black_pixel;             // XCB_CW_BACK_PIXEL

  cw_value_list[1] = XCB_EVENT_MASK_EXPOSURE          // XCB_CW_EVENT_MASK
                   | XCB_EVENT_MASK_KEY_PRESS
                   | XCB_EVENT_MASK_STRUCTURE_NOTIFY; // - for WM_DELETE_WINDOW

  xcb_create_window(
    connection,
    XCB_COPY_FROM_PARENT,                 // depth
    window,                               // window id
    screen->root,                         // parent window
    0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1, // x, y, width, height, border
    XCB_WINDOW_CLASS_INPUT_OUTPUT,        // class
    screen->root_visual,                  // visual
    cw_value_mask, cw_value_list          // config mask and values
  );



  // Set size hints for window managers
  // i3 ignores this

  xcb_intern_atom_cookie_t  normal_hints_cookie;
  xcb_intern_atom_reply_t  *normal_hints_reply;

  i32_t normal_hints[13] = { 0 };
  const char normal_hints_atom_name[] = "WM_NORMAL_HINTS";


  normal_hints_cookie = xcb_intern_atom(
    connection,
    0,
    sizeof(normal_hints_atom_name) - 1,
    normal_hints_atom_name
  );

  normal_hints_reply = xcb_intern_atom_reply(
    connection,
    normal_hints_cookie,
    NULL
  );

  if (!normal_hints_reply)
  {
    print_error("Cannot get atom WM_NORMAL_HINTS");
    return -1;
  }

  // flags: PMinSize and PMaxSize (bits: 4 and 5)
  normal_hints[0] = (1 << 4) | (1 << 5);

  // minimal window size
  normal_hints[5] = SCREEN_WIDTH;  // min_width
  normal_hints[6] = SCREEN_HEIGHT; // min_height

  // maximal window size
  normal_hints[7] = SCREEN_WIDTH;  // max_width
  normal_hints[8] = SCREEN_HEIGHT; // max_height

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    normal_hints_reply->atom,
    normal_hints_reply->atom,
    sizeof(i32_t) * 8, // bits in i32_t
    13, // elements in normal_hints[13]
    normal_hints
  );

  free(normal_hints_reply);
  normal_hints_reply = NULL;



  // Set WM_PROTOCOLS and WM_DELETE_WINDOW

  xcb_intern_atom_cookie_t  protocols_cookie;
  xcb_intern_atom_cookie_t  delete_cookie;
  xcb_intern_atom_reply_t  *protocols_reply;
  xcb_intern_atom_reply_t  *delete_reply;

  const char protocols_atom_name[] = "WM_PROTOCOLS";
  const char delete_atom_name[]    = "WM_DELETE_WINDOW";

  xcb_atom_t protocols[1];


  protocols_cookie = xcb_intern_atom(
    connection,
    0,
    sizeof(protocols_atom_name) - 1,
    protocols_atom_name
  );

  delete_cookie = xcb_intern_atom(
    connection,
    0,
    sizeof(delete_atom_name) - 1,
    delete_atom_name
  );


  protocols_reply = xcb_intern_atom_reply(
    connection,
    protocols_cookie,
    NULL
  );

  delete_reply = xcb_intern_atom_reply(
    connection,
    delete_cookie,
    NULL
  );


  if (!protocols_reply)
  {
    print_error("Cannot get WM_PROTOCOLS");
    return -1;
  }

  if (!delete_reply)
  {
    print_error("Cannot get WM_DELTE_WINDOW");
    return -1;
  }


  protocols[0] = delete_reply->atom;

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    protocols_reply->atom,
    XCB_ATOM_ATOM,
    sizeof(i32_t) * 8,
    1,
    protocols
  );

  free(protocols_reply);
  protocols_reply = NULL;

  // do not free delete_reply
  // we will use delete_reply->atom later



  // Set WM_CLASS

  xcb_intern_atom_cookie_t  wm_class_cookie;
  xcb_intern_atom_reply_t  *wm_class_reply;

  const char wm_class_atom_name[] = "WM_CLASS";
  const char wm_class[] = "xcbexample\0XCBExample\0";


  wm_class_cookie = xcb_intern_atom(
    connection,
    0,
    sizeof(wm_class_atom_name) - 1,
    wm_class_atom_name
  );

  wm_class_reply = xcb_intern_atom_reply(
    connection,
    wm_class_cookie,
    NULL
  );

  if (!wm_class_reply)
  {
    print_error("Cannot get atom WM_CLASS");
    return -1;
  }

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    wm_class_reply->atom, // use atom
    XCB_ATOM_STRING,      // data type
    sizeof(char) * 8,     // format (8 bit for char)
    sizeof(wm_class) - 1, // data length
    wm_class              // data
  );

  free(wm_class_reply);
  wm_class_reply = NULL;



  // Set WM_NAME

  xcb_intern_atom_cookie_t  wm_name_cookie;
  xcb_intern_atom_reply_t  *wm_name_reply;

  const char wm_name_atom_name[] = "WM_NAME";
  const char title[] = "XCB Example";


  wm_name_cookie = xcb_intern_atom(
    connection,
    0,
    sizeof(wm_name_atom_name) - 1,
    wm_name_atom_name
  );

  wm_name_reply = xcb_intern_atom_reply(
    connection,
    wm_name_cookie,
    NULL
  );

  if (!wm_name_reply)
  {
    print_error("Cannot get atom WM_NAME");
    return -1;
  }

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    window,
    wm_name_reply->atom,
    XCB_ATOM_STRING,
    sizeof(char) * 8,
    sizeof(title) - 1,
    title
  );

  free(wm_name_reply);
  wm_name_reply = NULL;



  // Show window and make changes visible

  xcb_map_window(connection, window);
  xcb_flush(connection);



  // Event Loop

  loop = true;


  print_string_endl("press ESC to EXIT");

  while (loop)
  {
    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event(connection)))
    {
      switch (event->response_type & ~0x80)
      {
        case XCB_EXPOSE:
        {
          xcb_expose_event_t *expose;
          expose = (xcb_expose_event_t *) event;
          (void) expose;
          break;
        }

        case XCB_KEY_PRESS:
        {
          xcb_key_press_event_t *key;

          key = (xcb_key_press_event_t *) event;

          if (key->detail == 9)
            loop = false;


          print_string("key: ");
          print_number(key->detail);
          print_endl();

          break;
        }

        case XCB_CLIENT_MESSAGE:
        {
          xcb_client_message_event_t *client;

          client = (xcb_client_message_event_t *) event;

          if (delete_reply && delete_reply->atom == client->data.data32[0])
          {
            loop = false;
            print_string_endl("Received WM_DELETE_WINDOW message");
          }

          break;
        }

        case XCB_DESTROY_NOTIFY:
        {
          loop = false;
          print_string_endl("Received DESTROY_NOTIFY");
          break;
        }

      } // switch
    } // while

    free(event);
    event = NULL;
  } // while



  // Disconnect from X-Server

  if (delete_reply)
  {
    free(delete_reply);
    delete_reply = NULL;
  }

  xcb_disconnect(connection);

  connection = NULL;
  screen = NULL;
  setup = NULL;
}
