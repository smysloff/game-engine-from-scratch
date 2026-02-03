
// file: examples/core/print_format.c

#include "core/core.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

typedef int (*printer_t)(const char *restrict format, ...);

typedef struct person_s
{
  int32_t  age;
  char    *name;
  char     class;
} person_t;

void
person_dump(person_t *p, printer_t print)
{
  print(
    "<person> { age: %d, name: \"%s\", class: '%c' }\n",
             p->age,  p->name,      p->class
  );
}

int
main(void)
{
  person_t p = { 41, "John Smith", 'e' };
  person_dump(&p, printf);
  person_dump(&p, print_format_compatible);
}
