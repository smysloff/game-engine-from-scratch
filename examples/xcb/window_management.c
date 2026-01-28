
// file: examples/xcb/window_management.c

#include "./gl.c"

void
key_press_handler(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_string("key: ");    // ================
  print_number(e->detail);  // log pressed keys
  print_endl();             // ================

  if (e->detail == KEY_ESC) // ESC to exit ====
    gl->loop = false;
}

void
gl_set_wm_class(
  gl_t *gl,
  char *instance,
  char *class
) {
  xcb_intern_atom_cookie_t cookie =
    xcb_intern_atom(gl->connection, 0, 8, "WM_CLASS");

  xcb_intern_atom_reply_t *reply =
    xcb_intern_atom_reply(gl->connection, cookie, NULL);

  if (!reply)
  {
    print_error("Cannot get atom WM_CLASS");
    return;
  }

  isize_t instance_length = string_length(instance);
  isize_t class_length = string_length(class);
  isize_t total_length = instance_length + 1 + class_length + 1;

  char *class_string = malloc(total_length);

  // string format: instance + '\0' + class + '\0'
  memory_copy(class_string, instance, instance_length);
  class_string[instance_length] = '\0';
  memory_copy(class_string, class, class_length);
  class_string[class_length] = '\0';

  xcb_change_property(
    gl->connection,
    XCB_PROP_MODE_REPLACE,
    gl->window,
    reply->atom,     // atom WM_CLASS
    XCB_ATOM_STRING, // string type
    8,               // 8 bit for char
    total_length,
    class_string
  );

  free(reply);
  free(class_string);
  xcb_flush(gl->connection);
}

int
main(void)
{
  gl_t gl;

  gl.key_press_handler = key_press_handler;
  print_string_endl("log: press ESC to exit");
  gl_create_window(&gl, 640, 480, NULL);

  //gl_set_wm_class(&gl, "windowmanagement", "WindowManagement");

  // set WM_CLASS

  xcb_intern_atom_cookie_t wm_class_cookie =
    xcb_intern_atom(gl.connection, 0, 8, "WM_CLASS");

  xcb_intern_atom_reply_t *wm_class_reply =
    xcb_intern_atom_reply(gl.connection, wm_class_cookie, NULL);

  if (!wm_class_reply)
  {
    print_error("Cannot get atom WM_CLASS");
    xcb_disconnect(gl.connection);
    return -1;
  }

  char wm_class[] = "xcb_example\0XCB_Example\0";

  xcb_change_property(
    gl.connection,
    XCB_PROP_MODE_REPLACE,
    gl.window,
    wm_class_reply->atom,
    XCB_ATOM_STRING,
    8,
    sizeof(wm_class) - 1,
    wm_class
  );

  free(wm_class_reply);


  // set WM_NAME

  xcb_intern_atom_cookie_t wm_name_cookie =
    xcb_intern_atom(gl.connection, 0, 7, "WM_NAME");

  xcb_intern_atom_reply_t *wm_name_reply =
    xcb_intern_atom_reply(gl.connection, wm_name_cookie, NULL);

  if (!wm_name_reply)
  {
    print_error("Cannot get atom WM_NAME");
    xcb_disconnect(gl.connection);
    return -1;
  }

  char title[] = "XCB Example Window";

  xcb_change_property(
    gl.connection,
    XCB_PROP_MODE_REPLACE,
    gl.window,
    wm_name_reply->atom,
    XCB_ATOM_STRING,
    8,
    sizeof(title) - 1,
    title
  );

  free(wm_name_reply);

  xcb_flush(gl.connection);

  gl_loop_frames(&gl);
  gl_destroy_window(&gl);
}
