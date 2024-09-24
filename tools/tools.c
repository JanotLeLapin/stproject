#include <stdint.h>
#include <stdio.h>

#include "tools.h"

int
main(void)
{
  FILE *in = stdin;
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
        return 1;
    }
    fprintf(stderr, "decoding failed: %s\n", err);
    return 1;
  }

  printf("BMG: inf entries: %d, flw instructions: %d, flw labels: %d, fli entries: %d\n\n", bmg.inf_entry_count, bmg.flw_instruction_count, bmg.flw_label_count, bmg.fli_entry_count);

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

    printf("%d %s\n\n\n", inf_entry.attributes, dat_entry);
  }

  for (i = 0; i < bmg.flw_instruction_count; i++) {
    flw_instruction = bmg_get_flw_instruction(&bmg, i);
    printf("%ld ", (unsigned long) flw_instruction);
  }

  printf("\n\n");

  for (i = 0; i < bmg.flw_label_count; i++) {
    flw_label = bmg_get_flw_label(&bmg, i);
    flw_id = bmg_get_flw_id(&bmg, i);
    printf("(%d, %d) ", flw_id, flw_label);
  }

  printf("\n\n");

  for (i = 0; i < bmg.fli_entry_count; i++) {
    fli_entry = bmg_get_fli_entry(&bmg, i);
    printf("(%d, %d) ", fli_entry.id, fli_entry.index);
  }

  bmg_free_file(&bmg);
}
