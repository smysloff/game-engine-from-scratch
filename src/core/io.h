
// file: src/core/io.h

#pragma once

#include "./types.h"

isize_t print_char_fd(char c, i32_t fd);
isize_t print_char(char c);
isize_t print_endl(void);
isize_t print_string_fd(const char *s, i32_t fd);
isize_t print_string(const char *s);
isize_t print_string_endl(const char *s);
isize_t print_log(const char *s);
isize_t print_error(const char *s);
isize_t print_number(i64_t n);


#ifdef CORE_IO_IMPLEMENTATION

#include "./dependencies.h"
#include "./util.h"

isize_t
print_char_fd(char c, i32_t fd)
{
  return write(fd, &c, 1);
}

isize_t
print_char(char c)
{
  return print_char_fd(c, 1);
}

isize_t
print_endl(void)
{
  return print_char('\n');
}

isize_t
print_string_fd(const char *s, i32_t fd)
{
  return write(fd, s, string_length(s));
}

isize_t
print_string(const char *s)
{
  return print_string_fd(s, 1);
}

isize_t
print_string_endl(const char *s)
{
  isize_t bytes = 0;

  bytes += print_string(s);
  bytes += print_endl();

  return bytes;
}

isize_t
print_log(const char *s)
{
  isize_t bytes = 0;

  bytes += print_string("log: ");
  bytes += print_string(s);
  bytes += print_endl();

  return bytes;
}

isize_t
print_error(const char *s)
{
  isize_t bytes = 0;
  const char prefix[] = "error: ";
  char endl = '\n';

  bytes += write(2, prefix, string_length(prefix));
  bytes += write(2, s, string_length(s));
  bytes += write(2, &endl, 1);

  return bytes;
}

isize_t
print_number(i64_t n)
{
  char s[22] = { 0 };
  isize_t left = 0;
  isize_t right = number_length(n) - 1;

  if (n == 0)
    return print_char('0');

  if (n < 0)
    s[left++] = '-';

  while (right >= left)
  {
    i64_t reminder = n % 10;
    s[right--] = absolute_number(reminder) + 48;
    n /= 10;
  }

  return print_string(s);
}

#endif
