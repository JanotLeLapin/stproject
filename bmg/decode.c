#include "bmg.h"

#include <stdio.h>
#include <stdlib.h>

struct Message {
  unsigned int offset;
  unsigned int data;
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

void
decode(void *in, void *out)
{
  char *buf = malloc(32);
  struct Message *messages;
  size_t i, j, buf_size = 33;
  unsigned char encoding, c;
  unsigned short message_count, data_size, inf_block_size;
  unsigned int file_size, section_count, inf_section_size, message_offset, message_data, dat_section_size;

  fread(buf, 1, 32, in);
  file_size = (unsigned int) read_int(buf + 8, 4);
  section_count = (unsigned int) read_int(buf + 12, 4);
  encoding = buf[16] - 1;
  // fprintf(out, "HEADER\nfile size: %d\nsections: %d\nencoding: %s\n", file_size, section_count, encodings[encoding]);

  fread(buf, 1, 16, in);
  inf_section_size = (unsigned int) read_int(buf + 4, 4);
  message_count = (unsigned short) read_int(buf + 8, 2);
  inf_block_size = (unsigned short) read_int(buf + 10, 2);
  // fprintf(out, "INF1\nsection size: %d\nmessages: %d\ndata size: %d\n", inf_section_size, message_count, inf_block_size);

  if (buf_size <= inf_section_size - 16) {
    buf_size = inf_section_size - 16;
    buf = realloc(buf, buf_size);
  }
  messages = malloc(8 * message_count);

  fread(buf, 1, inf_section_size - 16, in);
  for (i = 0; i < message_count; i++) {
    message_offset = (unsigned int) read_int(buf + i * inf_block_size, 4);
    messages[i] = (struct Message) {
      .offset = (unsigned int) read_int(buf + i * inf_block_size, 4),
      .data = (unsigned int) read_int(buf + i * inf_block_size + 4, 4),
    };
  }

  fread(buf, 1, 8, in);
  dat_section_size = (unsigned int) read_int(buf + 4, 4);
  // fprintf(out, "DAT1\nsection size: %d\n", dat_section_size);

  if (buf_size <= dat_section_size - 8) {
    buf_size = dat_section_size - 8;
    buf = realloc(buf, buf_size);
  }

  fread(buf, 1, dat_section_size - 8, in);
  for (i = 0; i < message_count; i++) {
    fprintf(out, "%d:", messages[i].data);
    j = messages[i].offset;
    while (0 != buf[j]) {
      c = (unsigned char) buf[j];

      switch (c) {
        case 0x0a:
          fprintf(out, "<br>");
          break;
        case 0x1a:
          fprintf(out, "<bin");
          size_t until = j + buf[j + 2];
          while (j < until) {
            fprintf(out, " %02x", (unsigned char) buf[j]);
            j++;
          }
          fprintf(out, ">");
          continue;
        case 0xe0:
          fprintf(out, "à");
          break;
        case 0xe2:
          fprintf(out, "â");
          break;
        case 0xe7:
          fprintf(out, "ç");
          break;
        case 0xe8:
          fprintf(out, "è");
          break;
        case 0xe9:
          fprintf(out, "é");
          break;
        case 0xea:
          fprintf(out, "ê");
          break;
        case 0xee:
          fprintf(out, "î");
          break;
        case 0xef:
          fprintf(out, "ï");
          break;
        case 0xf9:
          fprintf(out, "ù");
          break;
        default:
          fputc(buf[j], out);
          break;
      }

      j += 2;
    }
    fprintf(out, "\n");
  }

  free(messages);
  free(buf);
}
