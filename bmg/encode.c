#include "bmg.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Vec {
  void *ptr;
  size_t capacity, elem_size, vec_size;
};

struct Message {
  unsigned int flags;
  size_t start;
  size_t end;
};

struct Vec
new_vec(size_t initial_capacity, size_t elem_size)
{
  return (struct Vec) {
    .ptr = malloc(elem_size * initial_capacity),
    .capacity = initial_capacity,
    .elem_size = elem_size,
    .vec_size = 0,
  };
}

void
vec_push(struct Vec *vec, void *elem)
{
  if (vec->vec_size * vec->elem_size >= vec->capacity * vec->elem_size) {
    vec->capacity *= 2;
    vec->ptr = realloc(vec->ptr, vec->capacity * sizeof(struct Message));
  }

  memcpy(vec->ptr + vec->vec_size * vec->elem_size, elem, vec->elem_size);
  vec->vec_size++;
}

void *
vec_get(struct Vec *vec, size_t idx)
{
  return vec->ptr + vec->elem_size * idx;
}

void
write_int(char *buf, unsigned long value, char bytes)
{
  size_t i;

  for (i = 0; i < bytes; i++) {
    buf[i] = (value >> (i * 8)) & 0xFF;
  }
}

void
encode(void *in, void *out)
{
  char buf[256];
  char flags_buf[16];
  char c;
  size_t i, msg_i, start, total_size, inf_size, dat_size;
  struct Message msg;
  struct Vec messages = new_vec(128, sizeof(struct Message));
  struct Vec dat = new_vec(1024, 1);

  while (NULL != fgets(buf, 256, in)) {
    i = 0;
    while (':' != buf[i]) {
      flags_buf[i] = buf[i];
      i++;
    }
    flags_buf[i] = '\0';
    msg.flags = strtol(flags_buf, NULL, 10);

    i++;
    msg.start = dat.vec_size;
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
          vec_push(&dat, &c);
          vec_push(&dat, "\x00");
          i += 2;
          break;
        case '<':
          start = i;
          do {
            i++;
          } while (' ' != buf[i] && '>' != buf[i]);

          if (!strncmp(buf + start, "<br>", 4)) {
            vec_push(&dat, "\x0a");
            vec_push(&dat, "\x00");
            i++;
          } else if (!strncmp(buf + start, "<bin", 4)) {
            while ('>' != buf[i]) {
              flags_buf[0] = buf[i + 1];
              flags_buf[1] = buf[i + 2];
              flags_buf[2] = '\0';

              c = strtol(flags_buf, NULL, 16);
              vec_push(&dat, &c);

              i += 3;
              msg_i++;
            }
            i++;
          }

          break;
        default:
          vec_push(&dat, buf + i);
          vec_push(&dat, "\x00");
          i++;
          break;
      }
    }

    vec_push(&dat, "\x00");
    vec_push(&dat, "\x00");
    msg.end = dat.vec_size;
    vec_push(&messages, &msg);
  }

  dat_size = dat.vec_size + 22;
  inf_size = messages.vec_size * 8 + 32;
  total_size = dat_size + inf_size + 32;

  fwrite("MESGbmg1", 1, 8, out);
  write_int(buf, total_size, 4);
  fwrite(buf, 1, 4, out);
  write_int(buf, 2, 4);
  fwrite(buf, 1, 4, out);
  fwrite("\02", 1, 1, out);
  for (i = 0; i < 15; i++) { buf[i] = 0; }
  fwrite(buf, 1, 15, out);

  fwrite("INF1", 1, 4, out);
  write_int(buf, inf_size, 4);
  fwrite(buf, 1, 4, out);
  write_int(buf, messages.vec_size, 2);
  fwrite(buf, 1, 2, out);
  write_int(buf, 8, 2);
  fwrite(buf, 1, 2, out);
  fwrite("\4", 1, 1, out);
  for (i = 0; i < 3; i++) { buf[i] = 0; }
  fwrite(buf, 1, 3, out);

  start = 0;
  c = 0;
  for (i = 0; i < messages.vec_size; i++) {
    msg = *((struct Message *) vec_get(&messages, i));
    write_int(buf, start, 4);
    fwrite(buf, 1, 4, out);
    write_int(buf, msg.flags, 4);
    fwrite(buf, 1, 4, out);

    start += msg.end - msg.start;
    c = (c + 1) % 2;
  }
  for (i = 0; i < 24; i++) { buf[i] = 0; }
  fwrite(buf, 1, c ? 24 : 16, out);

  fwrite("DAT1", 1, 4, out);
  write_int(buf, dat_size, 4);
  fwrite(buf, 1, 4, out);

  c = 8;
  for (i = 0; i < messages.vec_size; i++) {
    msg = *((struct Message *) vec_get(&messages, i));
    fwrite(dat.ptr + msg.start, 1, msg.end - msg.start, out);
    c = (c + msg.end - msg.start) % 16;
  }

  for (i = 0; i < 16; i++) { buf[i] = 0; }

  c = (16 - c) % 16;
  if (c) {
    fwrite(buf, 1, c, out);
  }
  fwrite(buf, 1, 16, out);

  free(messages.ptr);
  free(dat.ptr);
}
