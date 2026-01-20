# PROJECT STRUCTURE

```sh
root/
├── .gitignore
├── README.md
├── Makefile
├── build/                    # Сборка (игнорируется в git)
├── docs/                     # Документация
├── examples/                 # Черновые примеры
│   ├── 01_xcb_window/
│   ├── 02_software_renderer/
│   ├── 03_event_loop/
│   └── ...
├── src/                      # Основной код
│   ├── core/                 # Независимые модули
│   │   ├── types.h           # Основные типы (u32, Vec2 и т.д.)
│   │   ├── math/             # Математика
│   │   ├── memory/           # Аллокаторы
│   │   ├── containers/       # Свои структуры данных
│   │   └── platform/         # Абстракции ОС (позже)
│   ├── platform/             # Платформозависимый код
│   │   ├── x11/              # X11/XCB реализация
│   │   ├── linux/            # Linux специфика
│   │   └── dummy/            # Заглушка для тестов
│   ├── graphics/             # Графика
│   │   ├── software/         # Программный рендерер
│   │   ├── gpu/              # GPU рендерер (позже)
│   │   └── api/              # Абстракция графического API
│   ├── input/                # Ввод
│   ├── audio/                # Звук (опционально)
│   ├── network/              # Сеть
│   └── game/                 # Игровая логика
└── tests/                    # Тесты
```
