
// file: src/core/containers/vector.h

#pragma once

#include "../types.h"

typedef struct
{
  usize_t capacity;
  usize_t count;
  usize_t item_size;
  void   *data;
} vector_t;

void    vector_free(vector_t *v);
bool_t  vector_init(vector_t *v, usize_t capacity, usize_t item_size);
bool_t  vector_resize(vector_t *v, usize_t new_capacity);
void *  vector_get(const vector_t *v, usize_t index);
usize_t vector_push(vector_t *v, void *value);
usize_t vector_shrink(vector_t *v);
usize_t vector_count_size(const vector_t *v);
usize_t vector_capacity_size(const vector_t *v);


#ifdef CORE_CONTAINERS_VECTOR_IMPLEMENTATION

#include "../dependencies.h"
#include "../io.h"
#include "../util.h"


static bool_t
is_overflow(const vector_t *v, usize_t capacity)
{
  return capacity > USIZE_MAX / v->item_size;
}

void
vector_free(vector_t *v)
{
  if (v)
  {
    free(v->data);
    v->capacity = 0;
    v->count = 0;
    v->item_size = 0;
    v->data = NULL;
  }
}

bool_t
vector_init(vector_t *v, usize_t capacity, usize_t item_size)
{
  if (v == NULL)
  {
    print_error("vector can't be NULL in vector_init()");
    return false;
  }

  if (item_size == 0)
  {
    print_error("item_size can't be 0 in vector_init()");
    return false;
  }

  v->capacity = capacity;
  v->count = 0;
  v->item_size = item_size;
  v->data = NULL;

  if (capacity > 0)
  {
    if (is_overflow(v, capacity))
    {
      print_error("size overflow in vector_init()");
      v->capacity = 0;
      return false;
    }

    v->data = malloc(item_size * capacity);

    if (v->data == NULL)
    {
      print_error("malloc error in vector_init()");
      v->capacity = 0;
      return false;
    }
  }

  return true;
}

bool_t
vector_resize(vector_t *v, usize_t new_capacity)
{
  if (!v)
    return false;

  if (
    v->capacity == v->count
    && v->capacity >= new_capacity
  ) {
    return false;
  }

  if (is_overflow(v, new_capacity))
  {
    print_error("size overflow in vector_resize()");
    return false;
  }

  usize_t new_size = new_capacity * v->item_size;
  void *new_data = realloc(v->data, new_size);

  if (!new_data)
  {
    print_error("realloc error in vector_resize()");
    return false;
  }

  v->data = new_data;
  v->capacity = new_capacity;

  return true;
}

void *
vector_get(const vector_t *v, usize_t index)
{
  if (
    v == NULL
    || index > v->count
    || index >= v->capacity
  ) {
    return NULL;
  }

  return ((char *) v->data) + index * v->item_size;
}

usize_t
vector_push(vector_t *v, void *value)
{
  if (!v || !value)
    return 0;

  //
  // If the count has reached capacity, then
  // - calculate new capacity,
  // - check it on overlow
  // - resize the vector
  //
  if (v->count == v->capacity)
  {
    usize_t new_capacity = v->capacity == 0 ? 2 : v->capacity * 2;

    if (new_capacity <= v->capacity)
    {
      print_error("size overflow in vector_push()");
      return v->count;
    }

    if (!vector_resize(v, new_capacity))
      return v->count;
  }

  //
  // Copy the memory from value pointer
  // to the first empty space in vector's memory
  //
  char *ptr = vector_get(v, v->count);
  memory_copy(ptr, value, v->item_size);
  v->count++;

  return v->count;
}

usize_t
vector_shrink(vector_t *v)
{
  if (v && v->capacity > v->count)
    vector_resize(v, v->count);

  return v->capacity;
}

usize_t
vector_count_size(const vector_t *v)
{
  return v->item_size * v->count;
}

usize_t
vector_capacity_size(const vector_t *v)
{
  return v->item_size * v->capacity;
}

#endif
