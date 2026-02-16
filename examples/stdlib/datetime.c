
// file: examples/stdlib/datetime.c


#define CORE_TIME_IMPLEMENTATION
#include "core/time.h"

#define CORE_IO_IMPLEMENTATION
#include "core/io.h"


int
main(void)
{
  const char *dt;
  dt = get_datetime(0, NULL);
  print_string_endl(dt);
  return 0;
}
