
// file: core/types.h

#pragma once

typedef signed char          i8_t;
typedef signed short        i16_t;
typedef signed int          i32_t;
typedef signed long long    i64_t;

typedef unsigned char        u8_t;
typedef unsigned short      u16_t;
typedef unsigned int        u32_t;
typedef unsigned long long  u64_t;

typedef i64_t isize_t;
typedef u64_t usize_t;

typedef _Bool bool_t;

#ifndef NULL
  #define NULL ((void *)0)
#endif

#ifndef SIZE_MAX
  #define SIZE_MAX 18446744073709551615UL
#endif

// @todo add more number constants
