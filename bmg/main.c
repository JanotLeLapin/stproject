#include "bmg.h"

#include <stdio.h>
#include <string.h>

static const char *usage = "usage: bmg encode out_file\n       bmg decode out_file";

int
main(int argc, char **argv)
{
  FILE *in = stdin, *out;

  if (argc <= 1) {
    fprintf(stderr, "expected operation\n\n%s\n", usage);
    return 1;
  }

  if (argc <= 2) {
    fprintf(stderr, "expected file name\n\n%s\n", usage);
    return 1;
  }

  out = fopen(argv[2], "w");

  if (!strcmp(argv[1], "encode")) {
    encode(in, out);
    fclose(out);
    return 0;
  }
  if (!strcmp(argv[1], "decode")) {
    decode(in, out);
    fclose(out);
    return 0;
  }

  fprintf(stderr, "invalid operation '%s'\n\n%s", argv[1], usage);
}
