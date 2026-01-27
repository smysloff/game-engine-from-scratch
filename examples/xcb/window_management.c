
// file: examples/xcb/window_management.c

#include "./gl.c"

void
key_press_handler(gl_t *gl)
{
  xcb_key_press_event_t *e = (xcb_key_press_event_t *) gl->event;

  print_string("key: ");    //
  print_number(e->detail);  // log pressed keys
  print_endl();             //

  if (e->detail == KEY_ESC) // ESC to exit
    gl->loop = false;
}

int
main(void)
{
  gl_t gl;

  gl.key_press_handler = key_press_handler;
  print_string_endl("log: press ESC to exit");

  gl_create_window(&gl, 640, 480, NULL);
  gl_loop_frames(&gl);
  gl_destroy_window(&gl);
}
