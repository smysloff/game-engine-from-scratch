
// file: examples/system/sysinfo.c

#define GLS_IMPLEMENTATION
#include "gls/gls.h"

#include <pwd.h>
#include <limits.h>

#include <xcb/xkb.h>


char *
smprintf(const char *fmt, ...)
{
  char *result;
  va_list args;
  int len;

  assert(fmt);

  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  result = malloc(++len);
  assert(result);

  va_start(args, fmt);
  vsnprintf(result, len, fmt, args);
  va_end(args);

  return result;
}

const char *
get_login(void)
{
  struct passwd *pw;
  pw = getpwuid(getuid());
  assert(pw && pw->pw_name);
  return pw->pw_name;
}

const char *
get_hostname(void)
{
  static char buffer[HOST_NAME_MAX + 1];
  int error;

  error = gethostname(buffer, HOST_NAME_MAX);
  assert(!error);

  return buffer;
}


const char *
get_keyboard_layout(gls_context_t *ctx)
{
  const char *layout_name;
  xcb_xkb_get_kbd_by_name_cookie_t cookie;
  xcb_xkb_get_kbd_by_name_reply_t *reply;

  assert(ctx);

  cookie = xcb_xkb_get_kbd_by_name(ctx->connection, XCB_XKB_ID_USE_CORE_KBD);
  *reply = xcb_xkb_get_kbd_by_name_reply(ctx->connection, cookie, NULL);
  assert(reply);

  layout_name = xcb_xkb_get_names_names(reply);
  free(reply);

  return layout_name;
}

//const char *
//get_keyboard_layout(void)
//{
//  static const char *layout = "ru";
//  return layout;
//}


int
main(void)
{
  char *sysinfo;

  const char *login;
  const char *hostname;
  const char *datetime;
  //const char *layout;
  int layout;

  gls_context_t ctx;


  gls_init(&ctx);

  login    = get_login();
  hostname = get_hostname();

  while (true)
  {
    datetime = get_datetime(0, NULL);
    layout   = get_keyboard_layout(&ctx);

    sysinfo = smprintf("%s@%s %d %s", login, hostname, layout, datetime);
    gls_set_root_window_name(&ctx, sysinfo);
    print_string_endl(sysinfo);

    free(sysinfo);
    sleep(1);
  }

  gls_quit(&ctx);

  return 0;
}
