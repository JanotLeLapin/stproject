#include <stdint.h>
#include <stdlib.h>
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
    fprintf(out, "%d %s\n", inf_entry.attributes, dat_entry);
  }

  fprintf(out, "\n");
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

static int
encode_bmg(FILE *in, FILE *out)
{
  char buf[1024], null_buf[64], start;
  struct Vec
    inf_vec = vec_init(512, sizeof(struct BmgInfEntry)),
    dat_vec = vec_init(1024, 1),
    flw_instruction_vec = vec_init(256, 8),
    flw_labels_vec = vec_init(256, 2),
    flw_id_vec = vec_init(256, 1),
    fli_vec = vec_init(64, sizeof(struct BmgFliEntry));
  struct BmgInfEntry inf;
  uint64_t flw_instruction;
  uint16_t flw_label;
  uint8_t flw_id;
  struct BmgFliEntry fli;
  size_t i, dat_size;

  for (i = 0; i < sizeof(null_buf); i++) {
    null_buf[i] = 0;
  }

  vec_append(&dat_vec, "\x00\x00", 2);
  while (fgets(buf, sizeof(buf), in)) {
    if ('\n' == buf[0]) {
      break;
    }

    inf.attributes = strtol(buf, NULL, 10);
    i = 1;
    while (' ' != buf[i - 1]) {
      i++;
    }

    // printf("%ld, %ld\n", i, strlen(buf) - 1 - i);
    if (1 == strlen(buf) - i) {
      inf.index = 0;
      vec_append(&inf_vec, &inf, 1);
    } else {
      inf.index = dat_vec.vec_size;
      vec_append(&inf_vec, &inf, 1);
      bmg_put_dat_entry(&dat_vec, buf + i, strlen(buf) - 1 - i);
    }
  }

  while (fgets(buf, sizeof(buf), in)) {
    if ('\n' == buf[0]) {
      break;
    }

    flw_instruction = strtol(buf, NULL, 10);
    vec_append(&flw_instruction_vec, &flw_instruction, 1);
  }

  while (fgets(buf, sizeof(buf), in)) {
    i = 0;

    if ('\n' == buf[0]) {
      break;
    }

    flw_id = strtol(buf + 1, NULL, 10);
    while (',' != buf[i]) {
      i++;
    }
    flw_label = strtol(buf + i + 2, NULL, 10);

    vec_append(&flw_id_vec, &flw_id, 1);
    vec_append(&flw_labels_vec, &flw_label, 1);
  }

  while (fgets(buf, sizeof(buf), in)) {
    i = 0;

    if ('\n' == buf[0]) {
      break;
    }

    fli.id = strtol(buf + 1, NULL, 10);
    while (',' != buf[i]) {
      i++;
    }
    fli.index = strtol(buf + i + 2, NULL, 10);

    vec_append(&fli_vec, &fli, 1);
  }

  dat_size = dat_vec.vec_size + 8;
  if (dat_size % 32) {
    dat_size += 32 - (dat_size % 32);
  }
  memcpy(buf, "DAT1", 4);
  memcpy(buf + 4, &dat_size, 4);
  fwrite(buf, 1, 8, out);
  fwrite(dat_vec.ptr, 1, dat_vec.vec_size, out);
  fwrite(null_buf, 1, dat_size - dat_vec.vec_size - 8, out);

  vec_free(&inf_vec);
  vec_free(&dat_vec);
  vec_free(&flw_instruction_vec);
  vec_free(&flw_labels_vec);
  vec_free(&flw_id_vec);
  vec_free(&fli_vec);

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
    if (encode) {
      return encode_bmg(in, out);
    } else {
      return decode_bmg(in, out);
    }
  } else {
    fprintf(stderr, "unknown format '%s'\n", argv[1]);
    return 1;
  }
}
