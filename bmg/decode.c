#include "bmg.h"

#include <stdio.h>
#include <stdlib.h>

struct Message {
  unsigned int offset;
  unsigned int data;
};

struct Context {
  FILE *in, *out;
  size_t buf_capacity;
  char *buf;
};

struct FileHeaders {
  unsigned int file_size;
  unsigned int section_count;
  unsigned char encoding;
};

struct InfSection {
  unsigned int section_size;
  unsigned short message_count;
  unsigned short message_size;
  unsigned int file_id;
  struct Message *messages;
};

long
read_int(char *buf, char bytes)
{
  long res = 0;
  char i;

  for (i = 0; i < bytes; i++) {
    res |= ((long) (unsigned char) buf[i]) << (8 * i);
  }

  return res;
}

struct FileHeaders
decode_headers(struct Context *ctx)
{
  fread(ctx->buf, 1, 32, ctx->in);

  return (struct FileHeaders) {
    .file_size = (unsigned int) read_int(ctx->buf + 8, 4),
    .section_count = (unsigned int) read_int(ctx->buf + 12, 4),
    .encoding = (unsigned char) ctx->buf[16] - 1,
  };
}

struct InfSection
decode_inf_section(struct Context *ctx)
{
  struct InfSection res;
  size_t i;

  fread(ctx->buf, 1, 16, ctx->in);
  res.section_size = (unsigned int) read_int(ctx->buf + 4, 4);
  res.message_count = (unsigned short) read_int(ctx->buf + 8, 2);
  res.message_size = (unsigned short) read_int(ctx->buf + 10, 2);
  res.file_id = (unsigned int) read_int(ctx->buf + 12, 4);

  if (ctx->buf_capacity <= res.section_size - 16) {
    ctx->buf_capacity = res.section_size - 16;
    ctx->buf = realloc(ctx->buf, ctx->buf_capacity);
  }

  fread(ctx->buf, 1, res.section_size - 16, ctx->in);

  res.messages = malloc(8 * res.message_count);
  for (i = 0; i < res.message_count; i++) {
    res.messages[i].offset = (unsigned int) read_int(ctx->buf + i * res.message_size, 4);
    res.messages[i].data = (unsigned int) read_int(ctx->buf + i * res.message_size + 4, 4);
  }

  return res;
}

unsigned int
decode_dat_section(struct Context *ctx, struct InfSection inf)
{
  size_t i, msg_i;
  unsigned int section_size;

  fread(ctx->buf, 1, 8, ctx->in);
  section_size = (unsigned int) read_int(ctx->buf + 4, 4);

  if (ctx->buf_capacity <= section_size - 8) {
    ctx->buf_capacity = section_size - 8;
    ctx->buf = realloc(ctx->buf, ctx->buf_capacity);
  }

  fread(ctx->buf, 1, section_size - 8, ctx->in);
  for (i = 0; i < inf.message_count; i++) {
    fprintf(ctx->out, "%d:", inf.messages[i].data);
    msg_i = inf.messages[i].offset;
    while (0 != ctx->buf[msg_i]) {
      switch ((unsigned char) ctx->buf[msg_i]) {
        case 0x0a:
          fprintf(ctx->out, "<br>");
          break;
        case 0x1a:
          fprintf(ctx->out, "<bin");
          size_t until = msg_i + ctx->buf[msg_i + 2];
          while (msg_i < until) {
            fprintf(ctx->out, " %02x", (unsigned char) ctx->buf[msg_i]);
            msg_i++;
          }
          fprintf(ctx->out, ">");
          continue;
        default:
          if ((unsigned char) ctx->buf[msg_i] >= 0xe0 && (unsigned char) ctx->buf[msg_i] <= 0xff) {
            fputc(0xc3, ctx->out);
            fputc((unsigned char) ctx->buf[msg_i] - 64, ctx->out);
          } else {
            fputc(ctx->buf[msg_i], ctx->out);
          }
          break;
      }

      msg_i += 2;
    }
    fputc('\n', ctx->out);
  }

  return section_size;
}

void
decode(void *in, void *out)
{
  struct Context ctx = {
    .buf = malloc(32),
    .buf_capacity = 32,
    .in = stdin,
    .out = out,
  };
  struct FileHeaders file_headers;
  struct InfSection inf_section;
  unsigned int dat_section_size;

  file_headers = decode_headers(&ctx);

  inf_section = decode_inf_section(&ctx);
  fprintf(ctx.out, "%d // file id\n\n", inf_section.file_id);

  dat_section_size = decode_dat_section(&ctx, inf_section);

  printf(
    "HEADER\nfile size: %d\nsections: %d\nencoding: %s\nINF1\nsection size: %d\nmessage count: %d\ndata size: %d\nDAT1\nsection size: %d\n",
    file_headers.file_size,
    file_headers.section_count,
    encodings[file_headers.encoding],
    inf_section.section_size,
    inf_section.message_count,
    inf_section.message_size,
    dat_section_size
  );

  free(inf_section.messages);
  free(ctx.buf);
}
