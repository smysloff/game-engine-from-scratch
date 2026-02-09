
// src/tui/tui.c

#pragma once

#include "../core/dependencies.h"
#include "../core/types.h"
#include "../core/util.h"

#define CORE_IO_IMPLEMENTATION
#include "../core/io.h"


typedef struct
{
  u32_t  width, height;
  u32_t  x_size, y_size;
  u32_t  byte_size;
  u32_t  border_size;
  u32_t  border_char;
  u8_t  *buffer;
} tui_window_t;


void tui_create_window(tui_window_t *window, u32_t width, u32_t height);
void tui_destroy_window(tui_window_t *window);
void tui_fill_window(tui_window_t *window, char c);
void tui_draw_window(tui_window_t *window, u32_t x_size, u32_t y_size);
void tui_set_border(tui_window_t *window, char c, u32_t size);
void tui_put_pixel(tui_window_t *window, char c, i32_t x, i32_t y);
void tui_draw_line(tui_window_t *window, char c, i32_t x0, i32_t y0, i32_t x1, i32_t y1);


#ifdef TUI_IMPLEMENTATION

static void
tui_draw_horizontal_border(tui_window_t *window, u32_t x_size, u32_t y_size)
{
  u32_t border_width = window->width + window->border_size * 2;

  for (u32_t r = 0; r < window->border_size; ++r) // how many rows?
  {
    for (u32_t y = 0; y < y_size; ++y)            // what is a pixel size by y?
    {
      for (u32_t c = 0; c < border_width; ++c)    // how many columns?
      {
        for (u32_t x = 0; x < x_size; ++x)        // what is a pixel size by x?
        {
          print_char(window->border_char);
        }
      }
      print_endl();
    }
  }
}

static void
tui_draw_vertical_border(tui_window_t *window, u32_t x_size)
{
  for (u32_t c = 0; c < window->border_size; ++c)   // how many columns?
  {
    for (u32_t x = 0; x < x_size; ++x)              // what is a pixel size by x?
    {
      print_char(window->border_char);
    }
  }
}


void
tui_create_window(tui_window_t *window, u32_t width, u32_t height)
{
  assert(window);
  window->border_size = 0;
  window->border_char = 0;
  window->width = width;
  window->height = height;
  window->byte_size = window->width * window->height;
  window->buffer = malloc(window->byte_size);
  assert(window->buffer);
}

void
tui_destroy_window(tui_window_t *window)
{
  assert(window);
  assert(window->buffer);
  free(window->buffer);
  window->buffer = NULL;
  window->byte_size = 0;
  window->width = 0;
  window->height = 0;
}

void
tui_fill_window(tui_window_t *window, char c)
{
  assert(window);
  for (u32_t i = 0; i < window->byte_size; ++i)
    window->buffer[i] = c;
}

void
tui_draw_window(tui_window_t *window, u32_t x_size, u32_t y_size)
{
  assert(window);

  if (x_size == 0) x_size = 1;
  if (y_size == 0) y_size = 1;

  tui_draw_horizontal_border(window, x_size, y_size);

  for (u32_t y = 0; y < window->height; ++y)
  {
    for (u32_t y_i = 0; y_i < y_size; ++y_i)
    {
      tui_draw_vertical_border(window, x_size);

      for (u32_t x = 0; x < window->width; ++x)
      {
        char c = window->buffer[y * window->width + x];
        for (u32_t x_i = 0; x_i < x_size; ++x_i)
        {
          print_char(c);
        }
      }

      tui_draw_vertical_border(window, x_size);
      print_endl();
    }
  }

  tui_draw_horizontal_border(window, x_size, y_size);
}

void
tui_set_border(tui_window_t *window, char c, u32_t size)
{
  assert(window);
  assert(!size || (size && is_print(c)));
  window->border_char = c;
  window->border_size = size;
}

void
tui_put_pixel(tui_window_t *window, char c, i32_t x, i32_t y)
{
  assert(window);
  assert(is_print(c));
  if (
       x >= 0 && x < (i32_t) window->width
    && y >= 0 && y < (i32_t) window->height
  ) {
    window->buffer[y * window->width + x] = c;
  }
}

void
tui_draw_line(tui_window_t *window, char c, i32_t x0, i32_t y0, i32_t x1, i32_t y1)
{
  assert(window);
  assert(is_print(c));

  i32_t dx  =  absolute_number(x1 - x0);
  i32_t dy  = -absolute_number(y1 - y0);
  i32_t sx  =  x0 < x1 ? 1 : -1;
  i32_t sy  =  y0 < y1 ? 1 : -1;
  i32_t err =  dx + dy;

  tui_put_pixel(window, c, x0, y0);

  while (1)
  {
    tui_put_pixel(window, c, x0, y0);
    if (x0 == x1 && y0 == y1) break;
    i32_t err2 = err * 2;
    if (err2 >= dy) { err += dy; x0 += sx; }
    if (err2 <= dx) { err += dx; y0 += sy; }
  }
}

#endif
