
// file: examples/core/vectors.c

#include "core/core.h"

int main(void)
{
  vector_t v;

  vector_init(&v, 16, sizeof(i32_t));

  for (i32_t i = 1; i < 10; ++i)
    vector_push(&v, &i);

  print_number(v.count);
  print_string(" ");
  print_number(v.capacity);
  print_string(" ");
  print_number(vector_count_size(&v));
  print_string(" ");
  print_number(vector_capacity_size(&v));
  print_endl();


  vector_shrink(&v);

  print_number(v.count);
  print_string(" ");
  print_number(v.capacity);
  print_string(" ");
  print_number(vector_count_size(&v));
  print_string(" ");
  print_number(vector_capacity_size(&v));
  print_endl();

  for (usize_t i = 0; i < v.count; ++i)
  {
    if (i > 0) print_char(' ');
    print_number(*(i32_t *)vector_get(&v, i));
  }
  print_endl();

  vector_free(&v);
}
