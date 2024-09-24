#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "tools.h"

struct Vec
vec_init(size_t initial_capacity, size_t elem_size)
{
  struct Vec vec;
  vec.ptr = malloc(initial_capacity * elem_size);
  vec.elem_size = elem_size;
  vec.vec_size = 0;
  vec.vec_capacity = initial_capacity;

  return vec;
}

void *
vec_get(struct Vec *vec, size_t idx)
{
  return vec->ptr + vec->elem_size * idx;
}

void *
vec_append(struct Vec *vec, void *elems, size_t elem_count)
{
  if (vec->vec_size + elem_count > vec->vec_capacity) {
    vec->vec_capacity = vec->vec_capacity * 2 + elem_count;
    vec->ptr = realloc(vec->ptr, vec->vec_capacity * vec->elem_size);
    if (!vec->ptr) {
      return NULL;
    }
  }

  memcpy(vec->ptr + vec->vec_size * vec->elem_size, elems, vec->elem_size * elem_count);
  vec->vec_size += elem_count;
  return vec->ptr;
}

void
vec_free(struct Vec *vec)
{
  free(vec->ptr);
}
