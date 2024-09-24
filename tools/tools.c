#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "tools.h"

static int
decode_bmg(FILE *in, FILE *out)
{
  enum BmgDecodeResult res;
  struct BmgFile bmg;
  size_t i;
  struct BmgInfEntry inf_entry;
  char dat_entry[1024];
  unsigned long flw_instruction;
  unsigned short flw_label;
  unsigned char flw_id;
  struct BmgFliEntry fli_entry;

  res = bmg_decode(&bmg, in);
  if (res) {
    char *err;
    switch (res) {
      case BMG_DECODE_READ_ERROR:
        err = "read error";
        break;
      case BMG_DECODE_INVALID_MAGIC:
        err = "invalid magic";
        break;
      case BMG_DECODE_INVALID_ENCODING:
        err = "invalid encoding";
        break;
      case BMG_DECODE_ALLOC_ERROR:
        err = "alloc error";
        break;
      case BMG_DECODE_INVALID_SECTION:
        err = "invalid section";
        break;
      case BMG_DECODE_INVALID_INF_ENTRY_SIZE:
        err = "invalid inf entry size";
        break;
      case BMG_DECODE_INVALID_FLI_ENTRY_SIZE:
        err = "invalid fli entry size";
        break;
      default:
        break;
    }
    fprintf(stderr, "decoding failed: %s\n", err);
    fclose(in);
    fclose(out);
    return 1;
  }

  printf("BMG: inf entries: %d, flw instructions: %d, flw labels: %d, fli entries: %d\n", bmg.inf_entry_count, bmg.flw_instruction_count, bmg.flw_label_count, bmg.fli_entry_count);

  /*
  for (i = 0; i < 3; i++) {
    inf_entry = bmg_get_inf_entry(&bmg, i);
    bmg_get_dat_entry(&bmg, inf_entry.index, dat_entry, 1024);
    flw_instruction = bmg_get_flw_instruction(&bmg, i);
    flw_label = bmg_get_flw_label(&bmg, i);
    flw_id = bmg_get_flw_id(&bmg, i);
    fli_entry = bmg_get_fli_entry(&bmg, i);

    printf("INF: index: %08x, attrs: %08x\n", inf_entry.index, inf_entry.attributes);
    printf("DAT: \"%s\"\n", dat_entry);
    printf("FLW: instruction: %ld, label: %04x, id: %02x\n", flw_instruction, flw_label, flw_id);
    printf("FLI: id: %08x, index: %04x\n\n", fli_entry.id, fli_entry.index);
  }
  */

  for (i = 0; i < bmg.inf_entry_count; i++) {
    inf_entry = bmg_get_inf_entry(&bmg, i);
    bmg_get_dat_entry(&bmg, inf_entry.index, dat_entry, 1024);

    fprintf(out, "%d %s\n\n\n", inf_entry.attributes, dat_entry);
  }

  for (i = 0; i < bmg.flw_instruction_count; i++) {
    flw_instruction = bmg_get_flw_instruction(&bmg, i);
    fprintf(out, "%ld\n", (unsigned long) flw_instruction);
  }

  fprintf(out, "\n");
  for (i = 0; i < bmg.flw_label_count; i++) {
    flw_label = bmg_get_flw_label(&bmg, i);
    flw_id = bmg_get_flw_id(&bmg, i);
    fprintf(out, "(%d, %d)\n", flw_id, flw_label);
  }

  fprintf(out, "\n");
  for (i = 0; i < bmg.fli_entry_count; i++) {
    fli_entry = bmg_get_fli_entry(&bmg, i);
    fprintf(out, "(%d, %d)\n", fli_entry.id, fli_entry.index);
  }

  bmg_free_file(&bmg);

  fclose(in);
  fclose(out);

  return 0;
}

int
main(int argc, char **argv)
{
  char encode;
  FILE *in, *out;

  if (argc < 5) {
    fprintf(stderr, "not enough arguments\n");
    return 1;
  }

  if (!strncmp("encode", argv[2], 6)) {
    encode = 1;
  } else if (!strncmp("decode", argv[2], 6)) {
    encode = 0;
  } else {
    fprintf(stderr, "unknown operation '%s'\n", argv[2]);
    return 1;
  }

  in = fopen(argv[3], "r");
  if (!in) {
    fprintf(stderr, "could not open '%s'\n", argv[3]);
    return 1;
  }

  out = fopen(argv[4], "w");
  if (!out) {
    fprintf(stderr, "could not open '%s'\n", argv[4]);
    fclose(in);
    return 1;
  }

  if (!strncmp("bmg", argv[1], 3)) {
    if (encode) {}
    else {
      return decode_bmg(in, out);
    }
  } else {
    fprintf(stderr, "unknown format '%s'\n", argv[1]);
    return 1;
  }
}
