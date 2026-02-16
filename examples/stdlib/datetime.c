
// file: examples/stdlib/datetime.c

#include <assert.h>
#include <xcb/xcb.h>


#define CORE_TIME_IMPLEMENTATION
#include "core/time.h"

#define CORE_IO_IMPLEMENTATION
#include "core/io.h"


void
gls_set_root_window_name(xcb_connection_t *connection, const char *name)
{
  assert(connection);
  assert(name);

  xcb_screen_t *screen;

  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  xcb_change_property(
    connection,
    XCB_PROP_MODE_REPLACE,
    screen->root,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    string_length(name),
    name
  );

  xcb_flush(connection);
}


int
main(void)
{
  xcb_connection_t *connection;
  const char *datetime;

  connection = xcb_connect(NULL, NULL);
  assert(connection);

  datetime = get_datetime(0, NULL);
  assert(datetime);

  print_string_endl(datetime);
  gls_set_root_window_name(connection, datetime);

  xcb_disconnect(connection);
  return 0;
}
