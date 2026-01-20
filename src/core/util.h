
// file: src/core/util.h

#pragma once

#include "./types.h"

static inline i64_t
absolute_number(i64_t n)
{
  return n < 0 ? -n : n; // @todo handle overflow
}

static inline usize_t
string_length(const char *s)
{
  usize_t len = 0;

  if (s)
    while (s[len])
      ++len;

  return len;
}

static inline usize_t
number_length(i64_t n)
{
  usize_t len = 0;

  if (n <= 0)
    ++len;

  for (; n != 0; ++len)
    n /= 10;

  return len;
}

static inline void *
memory_copy(void *dest, void *src, usize_t size)
{
  char *d_p = (char *) dest;
  char *s_p = (char *) src;

  if (d_p && s_p)
    for (usize_t i = 0; i < size; ++i)
      d_p[i] = s_p[i];

  return dest;
}
