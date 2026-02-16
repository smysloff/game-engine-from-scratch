
// file: src/core/time.h

#pragma once


#include "./dependencies.h"

const char *
get_datetime(time_t rawtime, const char *restrict fmt);


#ifdef CORE_TIME_IMPLEMENTATION

#include "./types.h"


static char datetime[128];

const char *
get_datetime(time_t rawtime, const char *fmt)
{
  struct tm *timeinfo;
  usize_t written_bytes;

  if (!rawtime)
    rawtime = time(NULL);

  timeinfo = localtime(&rawtime);

  written_bytes = strftime(
    datetime,
    sizeof(datetime),
    fmt ? fmt : "%c",
    timeinfo
  );

  written_bytes == 0
    ? ( datetime[0] = '\0' )
    : ( datetime[sizeof(datetime) - 1] = '\0' );

  return datetime;
}

#endif
