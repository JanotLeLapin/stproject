#include "bmg.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Message {
  unsigned int flags;
  char content[512];
};

struct MessageVec {
  struct Message *ptr;
  size_t capacity;
  size_t size;
};

void
push(struct MessageVec *vec, struct Message *msg)
{
  if (vec->size >= vec->capacity) {
    vec->capacity *= 2;
    vec->ptr = realloc(vec->ptr, vec->capacity * sizeof(struct Message));
  }

  vec->ptr[vec->size].flags = msg->flags;
  strcpy(vec->ptr[vec->size].content, msg->content);
  vec->size++;
}

void
encode(void *f)
{
  char buf[256];
  char flags_buf[16];
  size_t pos = 0, i, j;
  struct Message msg;
  struct MessageVec vec = {
    .ptr = malloc(sizeof(struct Message) * 128),
    .capacity = 128,
    .size = 0,
  };

  while (NULL != fgets(buf, 256, f)) {
    i = 0;
    while (':' != buf[i]) {
      flags_buf[i] = buf[i];
      i++;
    }
    flags_buf[i] = '\0';
    msg.flags = strtol(flags_buf, NULL, 10);

    i += 2;
    j = 0;
    while (1) {
      msg.content[j] = buf[i];

      if ('\\' == buf[i]) {
        msg.content[j + 1] = buf[i + 1];
        j++;
        i++;
      } else if ('"' == buf[i]) {
        break;
      }

      j++;
      i++;
    }

    msg.content[j] = '\0';

    push(&vec, &msg);
  }

  for (i = 0; i < vec.size; i++) {
    printf("%d: %s\n", vec.ptr[i].flags, vec.ptr[i].content);
  }

  free(vec.ptr);
}
