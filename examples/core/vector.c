
// file: examples/core/vectors.c

#include "core/core.h"

int
main(void)
{
  vector_t v;

  vector_init(&v, 16, sizeof(i32_t));

  for (i32_t i = 1; i < 10; ++i)
    vector_push(&v, &i);

  print_format_endl(
    "%d %d %d %d",
    v.count,
    v.capacity,
    vector_count_size(&v),
    vector_capacity_size(&v)
  );

  vector_shrink(&v);

  print_format_endl(
    "%d %d %d %d",
    v.count,
    v.capacity,
    vector_count_size(&v),
    vector_capacity_size(&v)
  );

  for (usize_t i = 0; i < v.count; ++i)
  {
    if (i > 0) print_char(' ');
    print_number(*(i32_t *)vector_get(&v, i));
  }
  print_endl();

  vector_free(&v);
}
