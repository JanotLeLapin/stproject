#include "bmg.h"

#include <stdio.h>
#include <string.h>

static const char *usage = "usage: bmg encode < file\n       bmg decode < file.bmg";

int
main(int argc, char **argv)
{
  if (argc <= 1) {
    fprintf(stderr, "expected operation\n\n%s\n", usage);
    return 1;
  }

  FILE *f = stdin;
  if (!strcmp(argv[1], "encode")) {
    encode(f);
    return 0;
  }
  if (!strcmp(argv[1], "decode")) {
    decode(f);
    return 0;
  }

  fprintf(stderr, "invalid operation '%s'\n\n%s", argv[1], usage);
}
