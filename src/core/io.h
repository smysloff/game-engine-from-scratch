
// file: src/core/io.h

#pragma once

#include "./types.h"

isize_t print_char(char c);
isize_t print_char_fd(i32_t fd, char c);
isize_t print_char_endl(char c);

isize_t print_endl(void);
isize_t print_endl_fd(i32_t fd);

isize_t print_number(i64_t n);
isize_t print_number_fd(i32_t fd, i64_t n);
isize_t print_number_endl(i64_t n);

isize_t print_string(const char *s);
isize_t print_string_fd(i32_t fd, const char *s);
isize_t print_string_endl(const char *s);

isize_t print_log(const char *fmt, ...);
isize_t print_log_fd(i32_t fd, const char *fmt, ...);
isize_t print_log_endl(const char *fmt, ...);

isize_t print_error(const char *fmt, ...);
isize_t print_error_fd(i32_t fd, const char *fmt, ...);
isize_t print_error_endl(const char *fmt, ...);

isize_t print_format(const char *fmt, ...);
isize_t print_format_fd(i32_t fd, const char *fmt, ...);
isize_t print_format_endl(const char *fmt, ...);

int printf_format(const char *restrict fmt, ...);
int dprintf_format(int fd, const char *restrict fmt, ...);


#ifdef CORE_IO_IMPLEMENTATION

#include "./dependencies.h"
#include "./util.h"


static isize_t
parse_format_fd(i32_t fd, va_list *args, const char *fmt)
{
  isize_t bytes = 0;

  i64_t  d;
  char   c;
  char  *s;

  if (fmt)
  {
    while (*fmt)
    {

      if (*fmt == '%')
      {
        ++fmt;

        switch (*fmt)
        {
          case 'd':
          {
            d = va_arg(*args, i64_t);
            bytes += print_number_fd(fd, d);
            break;
          }

          case 'c':
          {
            c = (char) va_arg(*args, i32_t);
            bytes += print_char_fd(fd, c);
            break;
          }

          case 's':
          {
            s = va_arg(*args, char *);
            bytes += print_string_fd(fd, s);
            break;
          }

          default:
          {
            print_string_fd(2, "error: Cannot process symbol ");
            print_char_fd(2, *fmt);
            print_endl_fd(2);
            exit(-1);
          }

        } // switch (*fmt)
      } // if (*fmt == '%')

      else
      {
        bytes += print_char_fd(fd, *fmt);
      }

      ++fmt;
    } // while (*fmt)
  } // if (fmt)

  return bytes;
}


isize_t
print_char(char c)
{
  return print_char_fd(1, c);
}

isize_t
print_char_fd(i32_t fd, char c)
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
  return print_char_fd(fd, '\n');
}


isize_t
print_number(i64_t n)
{
  return print_number_fd(1, n);
}

isize_t
print_number_fd(i32_t fd, i64_t n)
{
  char s[22] = { 0 };
  isize_t left = 0;
  isize_t right = number_length(n) - 1;

  if (n == 0)
    return print_char_fd(fd, '0');

  if (n < 0)
    s[left++] = '-';

  while (right >= left)
  {
    i64_t reminder = n % 10;
    s[right--] = absolute_number(reminder) + 48;
    n /= 10;
  }

  return print_string_fd(fd, s);
}

isize_t
print_number_endl(i64_t n)
{
  isize_t bytes = 0;

  bytes += print_number(n);
  bytes += print_endl();

  return bytes;
}


isize_t
print_string(const char *s)
{
  return print_string_fd(1, s);
}

isize_t
print_string_fd(i32_t fd, const char *s)
{
  return write(fd, s, string_length(s));
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
print_log(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 1;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "log: ");
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;
}

isize_t
print_log_fd(i32_t fd, const char *fmt, ...)
{
  va_list args;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "log: ");
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;

}

isize_t
print_log_endl(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 1;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "log: ");
  bytes += parse_format_fd(fd, &args, fmt);
  bytes += print_endl_fd(fd);
  va_end(args);

  return bytes;
}


isize_t
print_error(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 2;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "error: ");
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;
}

isize_t
print_error_fd(i32_t fd, const char *fmt, ...)
{
  va_list args;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "error: ");
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;
}

isize_t
print_error_endl(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 2;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += print_string_fd(fd, "error: ");
  bytes += parse_format_fd(fd, &args, fmt);
  bytes += print_endl_fd(fd);
  va_end(args);

  return bytes;
}


isize_t
print_format(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 1;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;
}

isize_t
print_format_fd(i32_t fd, const char *fmt, ...)
{
  va_list args;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += parse_format_fd(fd, &args, fmt);
  va_end(args);

  return bytes;
}

isize_t
print_format_endl(const char *fmt, ...)
{
  va_list args;
  i32_t fd = 1;
  isize_t bytes = 0;

  va_start(args, fmt);
  bytes += parse_format_fd(fd, &args, fmt);
  bytes += print_endl();
  va_end(args);

  return bytes;
}

int
print_format_compatible(const char *restrict fmt, ...)
{
  va_list args;
  int bytes = 0;
  i32_t fd = 1;

  va_start(args, fmt);
  bytes += (int) parse_format_fd(fd, &args, (const char *) fmt);
  va_end(args);

  return bytes;
}

int
print_format_fd_compatible(int fd, const char *restrict fmt, ...)
{
  va_list args;
  int bytes = 0;

  va_start(args, fmt);
  bytes += (int) parse_format_fd((i32_t) fd, &args, (const char *) fmt);
  va_end(args);

  return bytes;
}

#endif
