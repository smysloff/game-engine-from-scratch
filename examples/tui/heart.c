
// file: examples/tui/heart.c


#define TUI_IMPLEMENTATION
#include "tui/tui.h"


bool_t
point_in_circle(i32_t px, i32_t py, i32_t cx, i32_t cy, i32_t r)
{
  i32_t dx = px - cx;
  i32_t dy = py - cy;
  return dx*dx + dy*dy <= r*r;
}


int
main(void)
{
  const i32_t w = 39;
  const i32_t h = 19;

  tui_window_t win;

  tui_create_window(&win, w, h);
  tui_set_border(&win, '~', 2);
  tui_fill_window(&win, ' ');


  i32_t h1_3 = h/3;
  i32_t h2_3 = h - h1_3;
  i32_t radius = h1_3 + 1;
  i32_t left_cx = w/3;
  i32_t right_cx = w - left_cx - 1;
  i32_t cy = radius;


  for (i32_t y = 0; y < h; ++y)
  {
    for (i32_t x = 0; x < w; ++x)
    {

      bool_t in_left_circle  = point_in_circle(x, y, left_cx, cy, radius);
      bool_t in_right_circle = point_in_circle(x, y, right_cx, cy, radius);
      bool_t in_upper = (y <= cy + radius) && (in_left_circle || in_right_circle);

      // top
      if (y <= h1_3)
        if (in_upper)
          tui_put_pixel(&win, '@', x, y);

      // middle
      if (y >= h1_3 && y <= h2_3)
      {
        if (
             (x > left_cx  - radius + (y - h1_3))
          && (x < right_cx + radius - (y - h1_3))
        ) {
          tui_put_pixel(&win, '%', x, y);
        }
      }

      // bottom
      if (y > h2_3 && y < h)
      {
        if (
             (x >= left_cx  - radius + (y - (h1_3)) + 1)
          && (x <= right_cx + radius - (y - (h1_3)) - 1)
        ) {
          tui_put_pixel(&win, '%', x, y);
        }
      }

    }
  }


  tui_draw_window(&win, 2, 1);
  tui_destroy_window(&win);

  return 0;
}
