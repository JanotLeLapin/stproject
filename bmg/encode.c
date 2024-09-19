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
  size_t i, msg_i, start, total_size, inf_size, dat_size = 0;
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
            while ('>' != buf[i]) {
              flags_buf[0] = buf[i + 1];
              flags_buf[1] = buf[i + 2];
              flags_buf[2] = '\0';

              msg.content[msg_i] = strtol(flags_buf, NULL, 16);

              i += 3;
              msg_i++;
            }
            i++;
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
    dat_size += msg.length;
    push(&vec, &msg);
  }

  dat_size += 22;
  inf_size = vec.size * 8 + 32;
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
  write_int(buf, vec.size, 2);
  fwrite(buf, 1, 2, out);
  write_int(buf, 8, 2);
  fwrite(buf, 1, 2, out);
  fwrite("\4", 1, 1, out);
  for (i = 0; i < 3; i++) { buf[i] = 0; }
  fwrite(buf, 1, 3, out);

  start = 0;
  c = 0;
  for (i = 0; i < vec.size; i++) {
    write_int(buf, start, 4);
    fwrite(buf, 1, 4, out);
    write_int(buf, vec.ptr[i].flags, 4);
    fwrite(buf, 1, 4, out);

    start += vec.ptr[i].length;
    c = (c + 1) % 2;
  }
  for (i = 0; i < 24; i++) { buf[i] = 0; }
  fwrite(buf, 1, c ? 24 : 16, out);

  fwrite("DAT1", 1, 4, out);
  write_int(buf, dat_size, 4);
  fwrite(buf, 1, 4, out);

  c = 8;
  for (i = 0; i < vec.size; i++) {
    msg = vec.ptr[i];
    fwrite(msg.content, 1, msg.length, out);
    c = (c + msg.length) % 16;
  }

  for (i = 0; i < 16; i++) { buf[i] = 0; }

  c = (16 - c) % 16;
  if (c) {
    fwrite(buf, 1, c, out);
  }
  fwrite(buf, 1, 16, out);

  free(vec.ptr);
}
