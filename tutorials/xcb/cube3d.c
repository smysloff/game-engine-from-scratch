
// file: tutorials/xcb/cube3d.c

#define GLS_IMPLEMENTATION
#include "gls/gls.h"


int
main(void)
{
  gls_context_t ctx;
  gls_window_t  win;

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


  gls_init(&ctx);

  gls_create_window(&ctx, &win, window_width, window_height);
  gls_set_string_property(&ctx, &win, "WM_CLASS", wm_class, sizeof(wm_class) - 1);
  gls_set_string_property(&ctx, &win, "WM_NAME",  wm_name, 0);

  gls_show_window(&ctx, &win);
  gls_start(&ctx);

  while (gls_is_running(&ctx))
  {
    while ((ctx.event = xcb_poll_for_event(ctx.connection)))
    {
      switch (ctx.event->response_type & ~0x80)
      {
        case XCB_KEY_PRESS:
          xcb_key_press_event_t *key = (xcb_key_press_event_t *) ctx.event;
          if (key->detail == KEY_ESC) gls_stop(&ctx);
          break;
      }

      free(ctx.event);
    }

    gls_fill_window(&win, 0x000000);


    for (usize_t i = 0; i < array_length(edges); ++i)
    {
      i32_t idx0 = edges[i][0];
      i32_t idx1 = edges[i][1];
      i32_t x0   = gls_project_x(&win, vertices[idx0][0]);
      i32_t y0   = gls_project_y(&win, vertices[idx0][1]);
      i32_t x1   = gls_project_x(&win, vertices[idx1][0]);
      i32_t y1   = gls_project_y(&win, vertices[idx1][1]);
      gls_draw_line(&win, x0, y0, x1, y1, 0xFF00FF);
    }


    gls_blit_window(&ctx, &win);
  }

  gls_destroy_window(&ctx, &win);
  gls_quit(&ctx);

  return 0;
}
