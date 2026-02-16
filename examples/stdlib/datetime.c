
// file: examples/stdlib/datetime.c

#define CORE_TIME_IMPLEMENTATION
#include "core/time.h"

#define CORE_IO_IMPLEMENTATION
#include "core/io.h"

int
main(void)
{
  print_string_endl(get_datetime());
  return 0;
}
