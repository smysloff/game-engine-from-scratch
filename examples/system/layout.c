
// file: examples/system/layout.c
// compile: gcc layout.c `pkg-config --cflags --libs xcb xcb-xkb`

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>


char *strncpy_safe(char *dst, const char *src, size_t dsize);

xcb_connection_t *x11_connection_create(void);
void x11_connection_close(xcb_connection_t *connection);
bool x11_xkb_use_extension_init(xcb_connection_t *connection);

int get_keyboard_layout_group(xcb_connection_t *connection);
const char *get_keyboard_layout_name(xcb_connection_t *connection);


int
main(void)
{
  xcb_connection_t *connection;

  if (!(connection = x11_connection_create()))
    return EXIT_FAILURE;

  if (!x11_xkb_use_extension_init(connection))
    return EXIT_FAILURE;

  printf("keyboard layout: %s\n", get_keyboard_layout_name(connection));

  x11_connection_close(connection);
  return EXIT_SUCCESS;
}


char *
strncpy_safe(char *dst, const char *src, size_t dsize)
{
  if (dst && src && dsize)
  {
    strncpy(dst, src, dsize - 1);
    dst[dsize - 1] = '\0';
  }
  return dst;
}


xcb_connection_t *
x11_connection_create(void)
{
  xcb_connection_t *connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(connection))
  {
    perror("Cannot create new connection to X-Server");
    return NULL;
  }
  return connection;
}

void
x11_connection_close(xcb_connection_t *connection)
{
  if (connection)
    xcb_disconnect(connection);
}

bool
x11_xkb_use_extension_init(xcb_connection_t *connection)
{
  bool supported;
  xcb_xkb_use_extension_cookie_t  cookie;
  xcb_xkb_use_extension_reply_t  *reply;

  if (!connection)
    return false;

  cookie = xcb_xkb_use_extension(
    connection,
    XCB_XKB_MAJOR_VERSION,
    XCB_XKB_MINOR_VERSION
  );

  reply = xcb_xkb_use_extension_reply(connection, cookie, NULL);
  supported = reply && reply->supported;

  if (!supported)
    perror("XKB extension to X-Server is not supported");

  free(reply);
  return supported;
}

// returns: id of keyboard layout group
//   or -1 on failure
int
get_keyboard_layout_group(xcb_connection_t *connection)
{
  int group;

  xcb_xkb_get_state_cookie_t  state_cookie;
  xcb_xkb_get_state_reply_t  *state_reply;

  if (!connection)
  {
    perror("Connecion to X-Server is NULL");
    return -1;
  }

  state_cookie = xcb_xkb_get_state(connection, XCB_XKB_ID_USE_CORE_KBD);
  state_reply  = xcb_xkb_get_state_reply(connection, state_cookie, NULL);

  if (!state_reply)
  {
    perror("Cannot get keyboard layout group");
    return -1;
  }

  group = state_reply->group;
  free(state_reply);

  return group;
}


const char *
get_keyboard_layout_name(xcb_connection_t *connection)
{
  static char keyboard_layout[64];
  int group;

  xcb_xkb_get_names_cookie_t       names_cookie;
  xcb_xkb_get_names_reply_t       *names_reply;
  xcb_xkb_get_names_value_list_t  *value_list;
  xcb_atom_t                      *group_names;
  xcb_get_atom_name_cookie_t       atom_cookie;
  xcb_get_atom_name_reply_t       *atom_reply;
  uint8_t                         *group_names_base;


  // get keyboard layout group id

  group = get_keyboard_layout_group(connection);
  if (group == -1)
  {
    strncpy_safe(keyboard_layout, "unknown", sizeof(keyboard_layout));
    return keyboard_layout;
  }


  // get keyboard layout names list

  names_cookie = xcb_xkb_get_names(
    connection,
    XCB_XKB_ID_USE_CORE_KBD,
    XCB_XKB_NAME_DETAIL_GROUP_NAMES
  );

  names_reply = xcb_xkb_get_names_reply(connection, names_cookie, NULL);

  if (!names_reply)
  {
    perror("Cannot get keyboard layout names");
    strncpy_safe(keyboard_layout, "unknown", sizeof(keyboard_layout));
    return keyboard_layout;
  }


  // parse value_list to get atoms
  // get atom's name by group index
  // get atom's value as result

  value_list  = xcb_xkb_get_names_value_list(names_reply);

  //group_names_base = (uint8_t *) value_list
  //  + names_reply->nTypes       * sizeof(uint32_t)
  //  + names_reply->nIndicators  * sizeof(uint32_t)
  //  + names_reply->nKeys        * sizeof(uint32_t)
  //  + names_reply->nKeyAliases  * sizeof(xcb_xkb_key_alias_t)
  //  + names_reply->nRadioGroups * sizeof(uint8_t)
  //  + names_reply->nKTLevels    * sizeof(uint8_t)
  //  + names_reply->nGroups;    // other fields

  //group_names = (xcb_atom_t *) group_names_base;

  group_names = xcb_xkb_get_names_value_list_group_names(value_list);

  if (group_names && group < names_reply->nGroups)
  {
    if (group_names[group] != XCB_ATOM_NONE)
    {
      atom_cookie = xcb_get_atom_name(connection, group_names[group]);
      atom_reply  = xcb_get_atom_name_reply(connection, atom_cookie, NULL);

      if (atom_reply)
      {
        char *name   = (char *) xcb_get_atom_name_name(atom_reply);
        int name_len = xcb_get_atom_name_name_length(atom_reply);
        snprintf(keyboard_layout, sizeof(keyboard_layout), "%.*s", name_len, name);
        free(atom_reply);
      }

      else
        snprintf(keyboard_layout, sizeof(keyboard_layout), "%d", group);
    }

    else
      snprintf(keyboard_layout, sizeof(keyboard_layout), "%d", group);
  }

  else
    snprintf(keyboard_layout, sizeof(keyboard_layout), "%d", group);


  // EXIT SUCCESS

  free(names_reply);
  return keyboard_layout;
}
