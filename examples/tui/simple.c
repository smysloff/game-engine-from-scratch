
// file: examples/tui/simple.c

#define TUI_IMPLEMENTATION
#include "tui/tui.h"

int
main(void)
{
  tui_window_t win;

  tui_window_create(&win, 33, 15);
  tui_set_border(&win, '%', 1);
  tui_window_fill(&win, ' ');
  tui_put_pixel(&win, '@', 15, 9);
  tui_draw_line(&win, '#', 3, 2, 23, 6);
  tui_draw_line(&win, '?', 4, 11, 30, 7);
  tui_window_draw(&win, 0, 0);
  tui_window_destroy(&win);

  return 0;
}
