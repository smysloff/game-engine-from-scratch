
// file: src/core/io.h

#pragma once

#include "./types.h"

isize_t print_char(char c);
isize_t print_char_fd(char c, i32_t fd);
isize_t print_char_endl(char c);
isize_t print_endl(void);
isize_t print_endl_fd(i32_t fd);
isize_t print_number(i64_t n);
isize_t print_number_fd(i64_t n, i32_t fd);              // fd ?
isize_t print_string(const char *s);                     // fd 1
isize_t print_string_fd(const char *s, i32_t fd);        // fd ?
isize_t print_string_endl(const char *s);                // fd 1, '\n'
isize_t print_log(const char *s);                        // fd 1, '\n', 'log: '
isize_t print_log_fd(const char *s, i32_t fd);           // fd ?,       'log: '
isize_t print_error(const char *s);                      // fd 2, '\n', 'error: '
isize_t print_error_fd(const char *s, i32_t fd);         // fd ?,       'error: '
isize_t print_format(const char *fmt, ...);              //
isize_t print_format_fd(i32_t fd, const char *fmt, ...); //
isize_t print_format_endl(const char *fmt, ...);         //


#ifdef CORE_IO_IMPLEMENTATION

#include "./dependencies.h"
#include "./util.h"


isize_t
print_char(char c)
{
  return print_char_fd(c, 1);
}

isize_t
print_char_fd(char c, i32_t fd)
{
  return write(fd, &c, 1);
}

isize_t
print_char_endl(char c)
{
  isize_t bytes = 0;
  bytes += print_char(c);
  bytes += print_endl();
  return bytes;
}

isize_t
print_endl(void)
{
  return print_endl_fd(1);
}

isize_t
print_endl_fd(i32_t fd)
{
  return print_char_fd('\n', fd);
}

isize_t
print_number(i64_t n)
{
  return print_number_fd(n, 1);
}

isize_t
print_number_fd(i64_t n, i32_t fd)
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

  return print_string_fd(s, fd);
}

isize_t
print_log(const char *s)
{
  return print_log_fd(s, 1);
}

isize_t
print_log_fd(const char *s, i32_t fd)
{
  isize_t bytes = 0;

  bytes += print_string_fd("log: ", fd);
  bytes += print_string_fd(s, fd);
  bytes += print_endl_fd(fd);

  return bytes;
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

#endif
