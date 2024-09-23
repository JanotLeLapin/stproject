#include "tools.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INF_ENTRY_SIZE 8
#define INF_INDEX_SIZE 4
#define INF_ATTRIBUTES_SIZE 4

#define FLW_INSTRUCTION_SIZE 8
#define FLW_LABEL_SIZE 2
#define FLW_ID_SIZE 1

#define FLI_ENTRY_SIZE 8
#define FLI_ID_SIZE 4
#define FLI_INDEX_SIZE 2

unsigned long
read_int(char *buf, char bytes)
{
  long res = 0;
  char i;

  for (i = 0; i < bytes; i++) {
    res |= ((unsigned long) (unsigned char) buf[i]) << (8 * i);
  }

  return res;
}

enum BmgDecodeResult
bmg_decode(struct BmgFile *out, void *in)
{
  char headers[32];
  unsigned int sections, processed_sections = 0;
  size_t file_size, pos = 0;

  if (fread(headers, 1, 32, in) < 32) {
    return BMG_DECODE_READ_ERROR;
  }

  if (strncmp("MESGbmg1", headers, 8)) {
    return BMG_DECODE_INVALID_MAGIC;
  }
  if (2 != headers[16]) {
    return BMG_DECODE_INVALID_ENCODING;
  }

  file_size = read_int(headers + 8, 4) - 32;
  sections = read_int(headers + 12, 4);

  out->buf = malloc(file_size);
  if (!out->buf) {
    return BMG_DECODE_ALLOC_ERROR;
  }
  if (fread(out->buf, 1, file_size, in) < file_size) {
    free(out->buf);
    return BMG_DECODE_READ_ERROR;
  }

  out->inf_offset = 0;
  out->dat_offset = 0;
  out->flw_instructions_offset = 0;
  out->flw_labels_offset = 0;
  out->fli_offset = 0;
  while (processed_sections < sections) {
    if (!strncmp("INF1", out->buf + pos, 4)) {
      if (INF_ENTRY_SIZE != read_int(out->buf + pos + 10, 2)) {
        free(out->buf);
        return BMG_DECODE_INVALID_INF_ENTRY_SIZE;
      }
      out->inf_entry_count = read_int(out->buf + pos + 8, 2);
      out->inf_offset = pos + 16;
      pos += read_int(out->buf + pos + 4, 4);
    } else if (!strncmp("DAT1", out->buf + pos, 4)) {
      out->dat_offset = pos + 8;
      pos += read_int(out->buf + pos + 4, 4);
    } else if (!strncmp("FLW1", out->buf + pos, 4)) {
      out->flw_instruction_count = read_int(out->buf + pos + 8, 2);
      out->flw_label_count = read_int(out->buf + pos + 10, 2);
      out->flw_instructions_offset = pos + 16;
      out->flw_labels_offset = pos + 16 + out->flw_instruction_count * FLW_INSTRUCTION_SIZE;
      pos += read_int(out->buf + pos + 4, 4);
    } else if (!strncmp("FLI1", out->buf + pos, 4)) {
      if (FLI_ENTRY_SIZE != read_int(out->buf + pos + 10, 2)) {
        return BMG_DECODE_INVALID_FLI_ENTRY_SIZE;
      }
      out->fli_offset = pos + 16;
      out->fli_entry_count = read_int(out->buf + pos + 8, 2) + 16;
      pos += out->fli_entry_count * FLI_ENTRY_SIZE;
    } else {
      free(out->buf);
      return BMG_DECODE_INVALID_SECTION;
    }

    processed_sections++;
  }

  return BMG_DECODE_OK;
}

void
bmg_free_file(struct BmgFile *bmg)
{
  free(bmg->buf);
}

struct BmgInfEntry
bmg_get_inf_entry(struct BmgFile *bmg, unsigned short idx)
{
  struct BmgInfEntry inf;
  char *ptr = bmg->buf + bmg->inf_offset + idx * INF_ENTRY_SIZE;

  inf.index = read_int(ptr, INF_INDEX_SIZE);
  inf.attributes = read_int(ptr + INF_INDEX_SIZE, INF_ATTRIBUTES_SIZE);

  return inf;
}

void
bmg_get_dat_entry(struct BmgFile *bmg, size_t idx, char *res, size_t res_len)
{
  size_t buf_i = bmg->dat_offset + idx, res_i = 0;
  unsigned char c;

  while (res_i < res_len - 1) {
    c = bmg->buf[buf_i];
    if (!c) {
      break;
    }

    switch (c) {
      case 0x0a:
        res[res_i] = '\n';
        res_i++;
        buf_i += 2;
        break;
      case 0x1a:
        memcpy(res + res_i, "<bin", 4);
        res_i += 4;
        size_t until = bmg->buf[buf_i + 2] + buf_i;
        while (buf_i < until) {
          snprintf(res + res_i, 4, " %02x", (unsigned char) bmg->buf[buf_i]);
          res_i += 3;
          buf_i++;
        }
        res[res_i] = '>';
        res_i++;
        break;
      default:
        if (c >= 0xc0 && c <= 0xff) {
          res[res_i] = 0xc3;
          res[res_i + 1] = c - 64;
          res_i += 2;
          buf_i += 2;
          break;
        }

        res[res_i] = c;
        res_i++;
        buf_i += 2;
        break;
    }
  }

  res[res_i] = '\0';
}

unsigned long
bmg_get_flw_instruction(struct BmgFile *bmg, unsigned short idx)
{
  return read_int(bmg->buf + bmg->flw_instructions_offset + idx * FLW_INSTRUCTION_SIZE, FLW_INSTRUCTION_SIZE);
}

unsigned short
bmg_get_flw_label(struct BmgFile *bmg, unsigned short idx)
{
  return read_int(bmg->buf + bmg->flw_labels_offset + idx * FLW_LABEL_SIZE, FLW_LABEL_SIZE);
}

unsigned char
bmg_get_flw_id(struct BmgFile *bmg, unsigned short idx)
{
  return bmg->buf[bmg->flw_labels_offset + bmg->flw_label_count * FLW_LABEL_SIZE + idx];
}

struct BmgFliEntry
bmg_get_fli_entry(struct BmgFile *bmg, unsigned short idx)
{
  struct BmgFliEntry res;
  char *ptr = bmg->buf + bmg->fli_offset + idx * FLI_ENTRY_SIZE;

  res.id = read_int(ptr, FLI_ID_SIZE);
  res.index = read_int(ptr + FLI_ID_SIZE, FLI_INDEX_SIZE);

  return res;
}
