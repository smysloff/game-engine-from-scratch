
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static int entity_id;

typedef enum
{
  ENTITY_DEFAULT,
  ENTITY_PLAYABLE,
} entity_type_t;

typedef void (*logger_t)(void *);

void entity_logger(void *);
void playable_logger(void *);

typedef struct entity_s
{
  entity_type_t type;
  int id;
  float x, y;
  char title[32];
  logger_t log;
} entity_t;

void entity_init(entity_t *e, float x, float y, const char *title)
{
  assert(e != NULL);
  assert(title != NULL);

  e->type = ENTITY_DEFAULT;

  e->id = entity_id++;
  e->x = x;
  e->y = y;

  size_t len = sizeof(e->title) - 1;
  strncpy(e->title, title, len);
  e->title[len] = '\0';

  e->log = entity_logger;
}

void entity_logger(void *self)
{
  assert(self != NULL);
  entity_t *e = (entity_t *) self;
  assert(e->type != ENTITY_DEFAULT);

  printf("%s [%d] { %.02f, %.02f }\n", e->title, e->id, e->x, e->y);
}


typedef struct playable_s
{
  union {
    entity_t base;
    struct
    {
      entity_type_t type;
      int id;
      float x, y;
      char title[32];
      logger_t log;
    };
  };
  bool is_player;
  bool is_alive;
  int health;
} playable_t;

void playable_init(playable_t *e, float x, float y, const char *title, bool is_player)
{
  assert(e != NULL);
  assert(title != NULL);

  e->type = ENTITY_PLAYABLE;

  e->id = entity_id++;
  e->x = x;
  e->y = y;

  size_t len = sizeof(e->title) - 1;
  strncpy(e->title, title, len);
  e->title[len] = '\0';

  e->log = playable_logger;

  e->is_player = is_player;
  e->is_alive = true;
  e->health = 100;
}

void playable_logger(void *self)
{
  assert(self != NULL);
  playable_t *e = (playable_t *) self;
  assert(e->type != ENTITY_PLAYABLE);

  printf("%s [%d] { %.02f, %.02f } (%s)\n",
    e->title, e->id, e->x, e->y,
      e->is_player ? "player" : "npc");
}

int
main(void)
{
  playable_t e;
  playable_init(&e, 21, 42, "John Smith", true);
  e.log(&e);
}
