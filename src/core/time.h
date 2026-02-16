
// file: src/core/time.h

#pragma once

const char *
get_datetime(void);

#ifdef CORE_TIME_IMPLEMENTATION

#include "./dependencies.h"
#include "./types.h"

static char datetime[128];

const char *
get_datetime(void)
{
  time_t rawtime = time(NULL);
  struct tm *timeinfo = localtime(&rawtime);
  usize_t bytes = strftime(datetime, sizeof(datetime), "%c", timeinfo);

  bytes == 0
    ? ( datetime[0] = '\0' )
    : ( datetime[sizeof(datetime) - 1] = '\0' );

  return datetime;
}

#endif
