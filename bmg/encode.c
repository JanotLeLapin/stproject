#include "bmg.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Message {
  unsigned int flags;
  size_t length;
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
  vec->ptr[vec->size].length = msg->length;
  memcpy(vec->ptr[vec->size].content, msg->content, msg->length);
  vec->size++;
}

void
encode(void *in, void *out)
{
  char buf[256];
  char flags_buf[16];
  char c;
  size_t i, msg_i, start;
  struct Message msg;
  struct MessageVec vec = {
    .ptr = malloc(sizeof(struct Message) * 128),
    .capacity = 128,
    .size = 0,
  };

  while (NULL != fgets(buf, 256, in)) {
    i = 0;
    while (':' != buf[i]) {
      flags_buf[i] = buf[i];
      i++;
    }
    flags_buf[i] = '\0';
    msg.flags = strtol(flags_buf, NULL, 10);

    i++;
    msg_i = 0;
    while ('\n' != buf[i]) {
      switch ((unsigned char) buf[i]) {
        case 0xc3:
          switch ((unsigned char) buf[i + 1]) {
            case 0xa0:
              c = 0xe0;
              break;
            case 0xa9:
              c = 0xe9;
              break;
            case 0xaa:
              c = 0xea;
              break;
            default:
              c = 0x00;
              break;
          }
          msg.content[msg_i] = c;
          msg.content[msg_i + 1] = 0x00;
          msg_i += 2;
          i += 2;
          break;
        case '<':
          start = i;
          do {
            i++;
          } while (' ' != buf[i] && '>' != buf[i]);

          if (!strncmp(buf + start, "<br>", 4)) {
            msg.content[msg_i] = 0x0a;
            msg.content[msg_i + 1] = 0x00;
            i++;
            msg_i += 2;
          } else if (!strncmp(buf + start, "<bin", 4)) {
            // process bin
          }

          break;
        default:
          msg.content[msg_i] = buf[i];
          msg.content[msg_i + 1] = 0x00;
          i++;
          msg_i += 2;
          break;
      }
    }

    msg.content[msg_i] = 0x00;
    msg.content[msg_i + 1] = 0x00;
    msg.length = msg_i + 2;
    push(&vec, &msg);
  }

  // fprintf(out, "%ld\n", msg.length);
  // fwrite(msg.content, 1, msg.length, out);

  for (i = 0; i < vec.size; i++) {
    msg = vec.ptr[i];
    fwrite(msg.content, 1, msg.length, out);
  }

  free(vec.ptr);
}
