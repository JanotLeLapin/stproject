#include "bmg.h"

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

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
  char buf[BUFFER_SIZE];
  size_t i, j, k = 0;
  unsigned char encoding;
  unsigned short message_count, data_size, inf_block_size;
  unsigned int file_size, section_count, inf_section_size, message_offset, message_data;
  unsigned long *messages;

  fgets(buf, 33, f);
  file_size = (unsigned int) read_int(buf + 8, 4);
  section_count = (unsigned int) read_int(buf + 12, 4);
  encoding = buf[16] - 1;
  printf("HEADER\nfile size: %d\nsections: %d\nencoding: %s\n", file_size, section_count, encodings[encoding]);

  fgets(buf, 17, f);
  inf_section_size = (unsigned int) read_int(buf + 4, 4);
  message_count = (unsigned short) read_int(buf + 8, 2);
  inf_block_size = (unsigned short) read_int(buf + 10, 2);
  printf("INF\nsection size: %d\nmessages: %d\ndata size: %d\n", inf_section_size, message_count, inf_block_size);
  messages = malloc(4 * message_count);

  fgets(buf, message_count * inf_block_size + 1, f);
  for (i = 0; i < message_count; i++) {
    message_offset = read_int(buf + i * inf_block_size, 4);
    messages[i] = message_offset;
    printf("offset: %d\n", message_offset);
  }

  free(messages);
}
