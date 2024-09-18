#include "bmg.h"

#include <stdio.h>
#include <stdlib.h>

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
decode(void *f)
{
  char *buf = malloc(32);
  unsigned long *messages;
  size_t i, j, buf_size = 33;
  unsigned char encoding, c;
  unsigned short message_count, data_size, inf_block_size;
  unsigned int file_size, section_count, inf_section_size, message_offset, message_data, dat_section_size;

  fread(buf, 1, 32, f);
  file_size = (unsigned int) read_int(buf + 8, 4);
  section_count = (unsigned int) read_int(buf + 12, 4);
  encoding = buf[16] - 1;
  printf("HEADER\nfile size: %d\nsections: %d\nencoding: %s\n", file_size, section_count, encodings[encoding]);

  fread(buf, 1, 16, f);
  inf_section_size = (unsigned int) read_int(buf + 4, 4);
  message_count = (unsigned short) read_int(buf + 8, 2);
  inf_block_size = (unsigned short) read_int(buf + 10, 2);
  printf("INF1\nsection size: %d\nmessages: %d\ndata size: %d\n", inf_section_size, message_count, inf_block_size);

  if (buf_size <= inf_section_size - 16) {
    buf_size = inf_section_size - 16;
    buf = realloc(buf, buf_size);
  }
  messages = malloc(8 * message_count);

  fread(buf, 1, inf_section_size - 16, f);
  for (i = 0; i < message_count; i++) {
    message_offset = (unsigned int) read_int(buf + i * inf_block_size, 4);
    messages[i] = message_offset;
    printf("offset: %d\n", message_offset);
  }

  fread(buf, 1, 8, f);
  dat_section_size = (unsigned int) read_int(buf + 4, 4);
  printf("DAT1\nsection size: %d\n", dat_section_size);

  if (buf_size <= dat_section_size - 8) {
    buf_size = dat_section_size - 8;
    buf = realloc(buf, buf_size);
  }

  fread(buf, 1, dat_section_size - 8, f);
  for (i = 0; i < message_count; i++) {
    for (j = messages[i]; 0 != buf[j]; j += 2) {
      c = (unsigned char) buf[j];

      switch (c) {
        case 0x0a:
          printf("<br>");
          break;
        case 0xe0:
          printf("à");
          break;
        case 0xe8:
          printf("è");
          break;
        case 0xe9:
          printf("é");
          break;
        case 0xea:
          printf("ê");
          break;
        default:
          putchar(buf[j]);
          break;
      }
    }
    putchar('\n');
  }

  free(messages);
  free(buf);
}
