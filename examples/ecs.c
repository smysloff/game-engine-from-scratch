
// file: examples/ecs.c

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct uint32_list_s
{
  uint32_t value;
  struct uint32_list_s *next;
} uint32_list_t;

size_t
uint32_list_size(uint32_node_t *list)
{
  size_t size;

  for (size = 0; list != NULL; ++size)
    list = list->next;

  return size;
}

uint32_list_t *
uint32_list_push(uint32_t value, uint32_list_t *list) // push back (FIFO)
{
  uint32_list_t *node

  if (node = malloc(sizeof(uint32_list_t)))
  {
    node->value = value;
    node->next = NULL;
    if (list)
    {
      while (list->next)
        list = list->next;
      list->next = node;
    }
  }

  return node;
}

uin32_t
uint32_list_pop(uint32_list_t *list)
{
  uint32_t value;
  uint32_list_t *head;

  if (!list)
  {
    perror("list can't be null when pop it");
    exit(EXIT_FAILURE);
  }


}

void
uint32_list_free(unit32_list_t *list)
{
  uint32_list_t *curr;

  while (list)
  {
    curr = list;
    list = list->next;
    free(curr);
  }
}

static uint32_t last_id = 1; // 0 is for NULL value
static uint32_list_t *removed_ids;

int
main(void)
{

}
