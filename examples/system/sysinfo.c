
// file: examples/system/sysinfo.c

#define GLS_IMPLEMENTATION
#include "gls/gls.h"

char *
smprintf(const char *fmt, ...)
{
  char *result;
  va_list args;
  int len;

  assert(fmt);

  // Get total length of result string
  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  result = malloc(++len);
  assert(result);

  // Create result string and use length
  va_start(args, fmt);
  vsnprintf(result, len, fmt, args);
  va_end(args);

  return result;
}

typedef struct sysinfo_s
{
  char *login;
  char *hostname;
  char *os;
  char *kernel;
  char *cpu;
  char *memory;
  const char *datetime;
} sysinfo_t;

char *
get_login(void)
{
  return smprintf("%s", getlogin());
}

char *
get_hostname(void)
{
  char buffer[256];
  int error;

  error = gethostname(buffer, sizeof(buffer));
  assert(!error);

  return smprintf("%s", buffer);
}


int
main(void)
{
  // Variables
  sysinfo_t sysinfo;
  char *statusinfo;
  //char *fetchinfo;
  gls_context_t ctx;

  // Get user login
  sysinfo.login = get_login();
  print_string_endl(sysinfo.login);

  // Get hostname
  sysinfo.hostname = get_hostname();
  print_string_endl(sysinfo.hostname);

  // Connect to X-server, update WN_NAME and close connection
  gls_init(&ctx);

  while (true)
  {
    // Get DateTime
    sysinfo.datetime = get_datetime(0, NULL);
    assert(sysinfo.datetime);

    // Form StatusInfo string
    statusinfo = smprintf("DateTime: %s", sysinfo.datetime);
    assert(statusinfo);

    // Set StatusInfo to Root Window
    gls_set_root_window_name(&ctx, statusinfo);

    // Cleanup
    free(statusinfo);
    //free(fetchinfo);

    // Add delay
    sleep(1);
  }

  // Close connection and exit
  gls_quit(&ctx);
  free(sysinfo.login);
  free(sysinfo.hostname);

  return 0;
}
