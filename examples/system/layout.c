#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

int
main(void)
{
  xcb_connection_t *conn;
  xcb_screen_t *screen;
  int ret;
  int32_t device_id;
  struct xkb_context *ctx;
  struct xkb_keymap *keymap;
  struct xkb_state *state;

  // 1. Подключаемся к X серверу
  conn = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(conn))
  {
    fprintf(stderr, "Не удалось подключиться к X серверу\n");
    return 1;
  }

  // 2. Получаем первый экран
  screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

  // 3. Инициализируем XKB extension
  ret = xkb_x11_setup_xkb_extension(conn,
                                    XKB_X11_MIN_MAJOR_XKB_VERSION,
                                    XKB_X11_MIN_MINOR_XKB_VERSION,
                                    0, NULL, NULL, NULL, NULL);
  if (!ret)
  {
    fprintf(stderr, "Не удалось инициализировать XKB extension\n");
    xcb_disconnect(conn);
    return 1;
  }

  // 4. Получаем ID устройства core keyboard
  device_id = xkb_x11_get_core_keyboard_device_id(conn);
  if (device_id == -1)
  {
    fprintf(stderr, "Не удалось получить ID клавиатуры\n");
    xcb_disconnect(conn);
    return 1;
  }

  // 5. Создаем контекст xkbcommon
  ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!ctx)
  {
    fprintf(stderr, "Не удалось создать контекст xkbcommon\n");
    xcb_disconnect(conn);
    return 1;
  }

  // 6. Получаем текущую раскладку
  keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id,
                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (!keymap)
  {
    fprintf(stderr, "Не удалось получить раскладку\n");
    xkb_context_unref(ctx);
    xcb_disconnect(conn);
    return 1;
  }

  // 7. Создаем состояние
  state = xkb_x11_state_new_from_device(keymap, conn, device_id);
  if (!state)
  {
    fprintf(stderr, "Не удалось создать состояние\n");
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    xcb_disconnect(conn);
    return 1;
  }

  // 8. Получаем и выводим информацию о текущей раскладке
  const char *layout_name = xkb_keymap_layout_get_name(keymap, 0);
  printf("Текущая раскладка: %s\n", layout_name ? layout_name : "unknown");

/*
  // 9. Подписываемся на события изменения состояния
  uint32_t values[3] = { 0, 0, 0 };
  xcb_xkb_select_events(conn,
                        device_id,
                        XCB_XKB_EVENT_TYPE_STATE_NOTIFY, // what мы хотим получать
                        0,                               // clear
                        0,                               // select all
                        0, 0, values);

  xcb_flush(conn);

  printf("Ожидание событий клавиатуры... (Нажмите Ctrl+C для выхода)\n");

  // 10. Главный цикл обработки событий
  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(conn)))
  {
    uint8_t response_type = event->response_type & ~0x80;

    // Проверяем, является ли событие XKB событием
    if (response_type == XCB_XKB_STATE_NOTIFY)
    {
      xcb_xkb_state_notify_event_t *kb_event = (xcb_xkb_state_notify_event_t*) event;

      // Обновляем состояние
      xkb_state_update_mask(state,
                            kb_event->baseMods,
                            kb_event->latchedMods,
                            kb_event->lockedMods,
                            kb_event->baseGroup,
                            kb_event->latchedGroup,
                            kb_event->lockedGroup);

      // Получаем активную группу (раскладку)
      xkb_layout_index_t layout = xkb_state_serialize_layout(state, XKB_STATE_LAYOUT_EFFECTIVE);
      const char *new_layout = xkb_keymap_layout_get_name(keymap, layout);

      printf("Раскладка изменена: %s (группа %d)\n", new_layout ? new_layout : "unknown", layout);

      // Проверяем активные модификаторы
      if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
        printf("  Ctrl активен\n");

      if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
        printf("  Alt активен\n");
    }

    free(event);
  }
*/

  // 11. Очистка ресурсов
  xkb_state_unref(state);
  xkb_keymap_unref(keymap);
  xkb_context_unref(ctx);
  xcb_disconnect(conn);

  return 0;
}
