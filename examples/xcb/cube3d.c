
// file: examples/xcb/cube3d.c

#define GL_IMPLEMENTATION
#include "gl/gl.h"


int
main(void)
{
  gl_context_t gl;
  gl_window_t  win;

  const i32_t window_width  = 990;
  const i32_t window_height = 540;
  const char  wm_class[]    = "xcb_example\0XCB_Example\0";
  const char  wm_name[]     = "XCB Example";


  double vertices[][2] = {
  //{   x ,   y  }
    { -.65, -.35 }, // nw 0 front
    {  .35, -.35 }, // ne 1 -----
    {  .35,  .65 }, // se 2
    { -.65,  .65 }, // sw 3
    { -.35, -.65 }, // nw 4 back
    {  .65, -.65 }, // ne 5 -----
    {  .65,  .35 }, // se 6
    { -.35,  .35 }, // sw 8
  };

  i32_t edges[][2] = {
    { 0, 1 }, // nw - ne 0
    { 1, 2 }, // ne - se 1
    { 2, 3 }, // se - sw 2
    { 3, 0 }, // sw - nw 3

    { 4, 5 }, // nw - ne 4
    { 5, 6 }, // ne - se 5
    { 6, 7 }, // se - sw 6
    { 7, 4 }, // sw - nw 7

    { 0, 4 }, // side edges
    { 1, 5 },
    { 2, 6 },
    { 3, 7 },
  };


  gl_init(&gl);

  gl_create_window(&gl, &win, window_width, window_height);
  gl_set_string_property(&gl, &win, "WM_CLASS", wm_class, sizeof(wm_class) - 1);
  gl_set_string_property(&gl, &win, "WM_NAME", wm_name, 0);

  gl_show_window(&gl, &win);
  gl_start(&gl);

  while (gl_is_running(&gl))
  {
    while ((gl.event = xcb_poll_for_event(gl.connection)))
    {
      switch (gl.event->response_type & ~0x80)
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) gl.event;
          if (key->detail == KEY_ESC) gl_stop(&gl);
          break;
      }

      free(gl.event);
    }

    gl_fill_window(&win, 0x000000);


    for (usize_t i = 0; i < array_length(edges); ++i)
    {
      i32_t idx0 = edges[i][0];
      i32_t idx1 = edges[i][1];
      i32_t x0 = gl_project_x(&win, vertices[idx0][0]);
      i32_t y0 = gl_project_y(&win, vertices[idx0][1]);
      i32_t x1 = gl_project_x(&win, vertices[idx1][0]);
      i32_t y1 = gl_project_y(&win, vertices[idx1][1]);
      gl_draw_line(&win, x0, y0, x1, y1, 0xFF00FF);
    }


    gl_blit_window(&gl, &win);

  }

  gl_destroy_window(&gl, &win);
  gl_quit(&gl);

  return 0;
}
