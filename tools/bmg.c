#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

#define INF_ENTRY_SIZE 8
#define INF_INDEX_SIZE 4
#define INF_ATTRIBUTES_SIZE 4

#define FLW_INSTRUCTION_SIZE 8
#define FLW_LABEL_SIZE 2
#define FLW_ID_SIZE 1

#define FLI_ENTRY_SIZE 8
#define FLI_ID_SIZE 4
#define FLI_INDEX_SIZE 2

enum BmgDecodeResult
bmg_decode(struct BmgFile *out, void *in)
{
  char headers[32];
  uint32_t sections, processed_sections = 0;
  size_t file_size, pos = 0;
  uint16_t inf_entry_size, fli_entry_size;
  uint32_t section_size;

  if (fread(headers, 1, 32, in) < 32) {
    return BMG_DECODE_READ_ERROR;
  }

  if (strncmp("MESGbmg1", headers, 8)) {
    return BMG_DECODE_INVALID_MAGIC;
  }
  if (2 != headers[16]) {
    return BMG_DECODE_INVALID_ENCODING;
  }

  memcpy(&sections, headers + 12, 4);
  memcpy(&file_size, headers + 8, 4);
  file_size -= 32;

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
      memcpy(&inf_entry_size, out->buf + pos + 10, 2);
      if (INF_ENTRY_SIZE != inf_entry_size) {
        free(out->buf);
        return BMG_DECODE_INVALID_INF_ENTRY_SIZE;
      }
      memcpy(&out->inf_entry_count, out->buf + pos + 8, 2);
      out->inf_offset = pos + 16;
      memcpy(&section_size, out->buf + pos + 4, 4);
      pos += section_size;
    } else if (!strncmp("DAT1", out->buf + pos, 4)) {
      out->dat_offset = pos + 8;
      memcpy(&section_size, out->buf + pos + 4, 4);
      pos += section_size;
    } else if (!strncmp("FLW1", out->buf + pos, 4)) {
      memcpy(&out->flw_instruction_count, out->buf + pos + 8, 2);
      memcpy(&out->flw_label_count, out->buf + pos + 10, 2);
      out->flw_instructions_offset = pos + 16;
      out->flw_labels_offset = pos + 16 + out->flw_instruction_count * FLW_INSTRUCTION_SIZE;
      memcpy(&section_size, out->buf + pos + 4, 4);
      pos += section_size;
    } else if (!strncmp("FLI1", out->buf + pos, 4)) {
      memcpy(&fli_entry_size, out->buf + pos + 10, 2);
      if (FLI_ENTRY_SIZE != fli_entry_size) {
        return BMG_DECODE_INVALID_FLI_ENTRY_SIZE;
      }
      out->fli_offset = pos + 16;
      memcpy(&out->fli_entry_count, out->buf + pos + 8, 2);
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
bmg_get_inf_entry(struct BmgFile *bmg, uint16_t idx)
{
  struct BmgInfEntry res;
  memcpy(&res, bmg->buf + bmg->inf_offset + idx * INF_ENTRY_SIZE, sizeof(struct BmgInfEntry));
  return res;
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

uint64_t
bmg_get_flw_instruction(struct BmgFile *bmg, uint16_t idx)
{
  uint64_t res;
  memcpy(&res, bmg->buf + bmg->flw_instructions_offset + idx * FLW_INSTRUCTION_SIZE, sizeof(uint64_t));
  return res;
}

uint16_t
bmg_get_flw_label(struct BmgFile *bmg, uint16_t idx)
{
  uint16_t res;
  memcpy(&res, bmg->buf + bmg->flw_labels_offset + idx * FLW_LABEL_SIZE, sizeof(uint16_t));
  return res;
}

uint8_t
bmg_get_flw_id(struct BmgFile *bmg, uint16_t idx)
{
  return bmg->buf[bmg->flw_labels_offset + bmg->flw_label_count * FLW_LABEL_SIZE + idx];
}

struct BmgFliEntry
bmg_get_fli_entry(struct BmgFile *bmg, unsigned short idx)
{
  struct BmgFliEntry res;
  memcpy(&res, bmg->buf + bmg->fli_offset + idx * FLI_ENTRY_SIZE, sizeof(struct BmgFliEntry));
  return res;
}
