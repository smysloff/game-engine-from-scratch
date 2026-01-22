
// file: core/types.h

#pragma once


// Default C types

//...........................void;
//...........................char;
//............................int;
//..........................float;
//.........................double;


// Base integer types

typedef signed char          i8_t;
typedef signed short        i16_t;
typedef signed int          i32_t;
typedef signed long long    i64_t;

typedef unsigned char        u8_t;
typedef unsigned short      u16_t;
typedef unsigned int        u32_t;
typedef unsigned long long  u64_t;


// Types for size and difference

#ifdef __LP64__
  typedef i64_t isize_t;
  typedef u64_t usize_t;
#else
  typedef i32_t isize_t;
  typedef u32_t usize_t;
#endif


// Boolean type

typedef _Bool bool_t;

#ifndef true
  #define true ((bool_t) 1)
#endif

#ifndef false
  #define false ((bool_t) 0)
#endif


// NULL

#ifndef NULL
  #define NULL ((void *) 0)
#endif


// Integer constants

#ifndef CHAR_MIN
  #define CHAR_MIN (-128)
#endif

#ifndef CHAR_MAX
  #define CHAR_MAX 127
#endif

#ifndef UCHAR_MAX
  #define UCHAR_MAX 255
#endif


// 8-bit

#ifndef I8_MIN
  #define I8_MIN (-128)
#endif

#ifndef I8_MAX
  #define I8_MAX 127
#endif

#ifndef U8_MAX
  #define U8_MAX 255
#endif


// 16-bit

#ifndef I16_MIN
  #define I16_MIN (-32768)
#endif

#ifndef I16_MAX
  #define I16_MAX 32767
#endif

#ifndef U16_MAX
  #define U16_MAX 65535
#endif


// 32-bit

#ifndef I32_MIN
  #define I32_MIN (-2147483648)
#endif

#ifndef I32_MAX
  #define I32_MAX 2147483647
#endif

#ifndef U32_MAX
  #define U32_MAX 4294967295U
#endif


// 64-bit

#ifndef I64_MIN
  #define I64_MIN (-9223372036854775808L)
#endif

#ifndef I64_MAX
  #define I64_MAX 9223372036854775807L
#endif

#ifndef U64_MAX
  #define U64_MAX 18446744073709551615UL
#endif


// Sizes and differences

#ifdef __LP64__

  #ifndef ISIZE_MIN
    #define ISIZE_MIN I64_MIN
  #endif

  #ifndef ISIZE_MAX
    #define ISIZE_MAX I64_MAX
  #endif

  #ifndef USIZE_MAX
    #define USIZE_MAX U64_MAX
  #endif

#else

  #ifndef ISIZE_MIN
    #define ISIZE_MIN I32_MIN
  #endif

  #ifndef ISIZE_MAX
    #define ISIZE_MAX I32_MAX
  #endif

  #ifndef USIZE_MAX
    #define USIZE_MAX U32_MAX
  #endif

#endif


// Character types

static inline bool_t
is_alnum(int c)
{
  return (c >= 48 && c <= 57)   // 0..9
      || (c >= 65 && c <= 90)   // A..Z
      || (c >= 97 && c <= 122); // a..z
}

static inline bool_t
is_alpha(int c)
{
  return (c >= 65 && c <= 90)   // A..Z
      || (c >= 97 && c <= 122); // a..z
}

static inline bool_t
is_cntrl(int c)
{
  return (c >= 0 && c <= 31)
      || (c == 127);
}

static inline bool_t
is_digit(int c)
{
  return c >= 48 && c <= 57;    // 0..9
}

static inline bool_t
is_graph(int c)
{
  return c >= 33 && c <= 126;
}

static inline bool_t
is_lower(int c)
{
  return c >= 97 && c <= 122;   // a..z
}

static inline bool_t
is_print(int c)
{
  return c >= 32 && c <= 126;
}

static inline bool_t
is_punct(int c)
{
  return (c >=  33 && c <=  47)  // !"#$%&'()*+,-./
      || (c >=  58 && c <=  64)  // :;<=>?@
      || (c >=  91 && c <=  96)  // [\]^_`
      || (c >= 123 && c <= 126); // {|}~
}

static inline bool_t
is_space(int c)
{
  return (c >= 9 && c <= 13)      // \t \n \v \f \r
      || (c == 32);               // space
}

static inline bool_t
is_upper(int c)
{
  return c >= 65 && c <= 90;      // A..Z
}

static inline bool_t
is_xdigit(int c)
{
  return (c >= 48 && c <= 57)     // 0..9
      || (c >= 65 && c <= 70)     // A..F
      || (c >= 97 && c <= 102);   // a..f
}

static inline bool_t
is_ascii(int c)
{
  return c >= 0 && c <= 127;
}

static inline bool_t
is_blank(int c)
{
  return c == 9 || c == 32;       // \t space
}
