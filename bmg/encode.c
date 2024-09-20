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

struct Context {
  FILE *in, *out;
  size_t i;
  char buf[1024];
  struct Vec inf;
  struct Vec dat;
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

unsigned int
encode_file_id(struct Context *ctx)
{
  char id_buf[16];
  size_t i = 0;

  while (' ' != ctx->buf[i]) {
    id_buf[i] = ctx->buf[i];
    i++;
  }
  id_buf[i] = '\0';
  return (unsigned int) strtol(id_buf, NULL, 10);
}

unsigned int
encode_flags(struct Context *ctx)
{
  char flags_buf[32];
  size_t start = ctx->i;

  while (':' != ctx->buf[ctx->i]) {
    flags_buf[ctx->i - start] = ctx->buf[ctx->i];
    ctx->i++;
  }
  flags_buf[ctx->i - start] = '\0';
  ctx->i++;
  return (unsigned int) strtol(flags_buf, NULL, 10);
}

void
encode_text(struct Context *ctx)
{
  char buf[3];
  unsigned char c;

  while ('\n' != ctx->buf[ctx->i]) {
    switch ((unsigned char) ctx->buf[ctx->i]) {
      case 0xc3:
        c = ctx->buf[ctx->i + 1] + 64;
        vec_push(&ctx->dat, &c);
        vec_push(&ctx->dat, "\x00");
        ctx->i += 2;
        break;
      case '<':
        if (!strncmp(ctx->buf + ctx->i, "<br>", 4)) {
          ctx->i += 4;
          vec_push(&ctx->dat, "\x0a");
          vec_push(&ctx->dat, "\x00");
        } else if (!strncmp(ctx->buf + ctx->i, "<bin", 4)) {
          ctx->i += 4;
          while ('>' != ctx->buf[ctx->i]) {
            buf[0] = ctx->buf[ctx->i + 1];
            buf[1] = ctx->buf[ctx->i + 2];
            buf[2] = '\0';

            c = strtol(buf, NULL, 16);
            vec_push(&ctx->dat, &c);

            ctx->i += 3;
          }
          ctx->i++;
        }

        break;
      default:
        vec_push(&ctx->dat, ctx->buf + ctx->i);
        vec_push(&ctx->dat, "\x00");
        ctx->i++;
        break;
    }
  }

  vec_push(&ctx->dat, "\x00");
  vec_push(&ctx->dat, "\x00");
}

void
encode(void *in, void *out)
{
  struct Context ctx = {
    .in = in,
    .out = out,
    .inf = new_vec(256, sizeof(struct Message)),
    .dat = new_vec(1024, 1),
  };
  char buf[256];
  char c;
  unsigned int file_id;
  size_t i, start, total_size, inf_size, dat_size;
  struct Message msg;

  fgets(ctx.buf, 1024, in);
  file_id = encode_file_id(&ctx);
  fgets(ctx.buf, 1024, in);
  while (NULL != fgets(ctx.buf, 1024, in)) {
    ctx.i = 0;
    msg.flags = encode_flags(&ctx);

    msg.start = ctx.dat.vec_size;
    encode_text(&ctx);
    msg.end = ctx.dat.vec_size;
    vec_push(&ctx.inf, &msg);
  }

  dat_size = ctx.dat.vec_size + 6;
  if (dat_size % 16) {
    dat_size += 16 - (dat_size % 16);
  } else {
    dat_size += 16;
  }

  inf_size = ctx.inf.vec_size * 8 + 32;
  if (ctx.inf.vec_size % 2) {
    inf_size += 8;
  }
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
  write_int(buf, ctx.inf.vec_size, 2);
  fwrite(buf, 1, 2, out);
  write_int(buf, 8, 2);
  fwrite(buf, 1, 2, out);
  write_int(buf, file_id, 4);
  fwrite(buf, 1, 4, out);

  start = 0;
  for (i = 0; i < ctx.inf.vec_size; i++) {
    msg = *((struct Message *) vec_get(&ctx.inf, i));
    write_int(buf, start, 4);
    fwrite(buf, 1, 4, out);
    write_int(buf, msg.flags, 4);
    fwrite(buf, 1, 4, out);

    start += msg.end - msg.start;
  }
  for (i = 0; i < 32; i++) { buf[i] = 0; }
  fwrite(buf, 1, inf_size - ctx.inf.vec_size * 8 - 16, out);

  fwrite("DAT1", 1, 4, out);
  write_int(buf, dat_size, 4);
  fwrite(buf, 1, 4, out);

  for (i = 0; i < ctx.inf.vec_size; i++) {
    msg = *((struct Message *) vec_get(&ctx.inf, i));
    fwrite(ctx.dat.ptr + msg.start, 1, msg.end - msg.start, out);
  }
  for (i = 0; i < 32; i++) { buf[i] = 0; }
  fwrite(buf, 1, dat_size - ctx.dat.vec_size - 8, out);

  free(ctx.inf.ptr);
  free(ctx.dat.ptr);
}
