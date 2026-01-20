# XCB LIBS


## Установка

```sh
sudo pacman -S \
  libxcb \
  xcb-proto \
  xcb-util \
  xcb-util-wm \
  xcb-util-keysyms \
  xcb-util-image \
  xcb-util-renderutil \
  xcb-util-cursor \
  xcb-util-errors
```

## Описание пакетов

| Пакет               | Размер  | Обязателен?   | Для чего                                                            |
|---------------------|---------|---------------|---------------------------------------------------------------------|
| libxcb              | ~1.5 MB | ДА            | Базовая библиотека                                                  |
| xcb-util            | ~200 KB | ДА            | Базовые утилиты: упрощенное создание окон, работа с экранами, цвета |
| xcb-util-keysyms    | ~50  KB | ДА            | Работа с клавиатурой: преобразование кодов клавиш в символы         |
| xcb-util-wm         | ~100 KB | Рекомендуется | Взаиможействие с оконными менеджером                                |
| xcb-util-image      | ~80  KB | Опционально   | Работа с изображениями                                              |
| xcb-util-cursor     | ~30  KB | Опционально   | Работа с курсорами мыши                                             |
| xcb-util-renderutil | ~60  KB | Опционально   | Утилиты для рендеринга: градиенты, треугольники и т.д.              |
| xcb-util-errors     | ~40  KB | Опционально   | Улучшенная обработка ошибок                                         |
| xcb-proto           | ~500 KB | Опционально   | XML описание протоколов X11                                         |


## struct xcb_screen_t

```c
typedef struct {
  xcb_window_t   root;
  xcb_colormap_t default_colormap;
  uint32_t       white_pixel;
  uint32_t       black_pixel;
  uint32_t       current_input_masks;
  uint16_t       width_in_pixels;
  uint16_t       height_in_pixels;
  uint16_t       width_in_millimeters;
  uint16_t       height_in_millimeters;
  uint16_t       min_installed_maps;
  uint16_t       max_installed_maps;
  xcb_visualid_t root_visual;
  uint8_t        backing_stores;
  uint8_t        save_unders;
  uint8_t        root_depth;
  uint8_t        allowed_depths_len;
} xcb_screen_t;
```
