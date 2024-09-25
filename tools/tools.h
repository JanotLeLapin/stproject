/* UTIL */
struct Vec {
  void *ptr;
  size_t elem_size;
  size_t vec_size;
  size_t vec_capacity;
};

struct Vec vec_init(size_t initial_capacity, size_t elem_size);
void *vec_get(struct Vec *vec, size_t idx);
void *vec_append(struct Vec *vec, void *elems, size_t elem_count);
void vec_free(struct Vec *vec);

/* BMG */
enum BmgDecodeResult {
  BMG_DECODE_OK = 0,

  BMG_DECODE_READ_ERROR,
  BMG_DECODE_INVALID_MAGIC,
  BMG_DECODE_INVALID_ENCODING,
  BMG_DECODE_ALLOC_ERROR,
  BMG_DECODE_INVALID_SECTION,

  BMG_DECODE_INVALID_INF_ENTRY_SIZE,
  BMG_DECODE_INVALID_FLI_ENTRY_SIZE,
};

struct BmgInfEntry {
  uint32_t index;
  uint32_t attributes;
} __attribute__((packed));

struct BmgFlwLabel {
  uint16_t label;
  uint8_t id;
} __attribute__((packed));

struct BmgFliEntry {
  uint32_t id;
  uint16_t index;
} __attribute__((packed));

struct BmgFile {
  char *buf;
  size_t inf_offset, dat_offset, flw_instructions_offset, flw_labels_offset, fli_offset;
  uint16_t inf_entry_count, flw_instruction_count, flw_label_count, fli_entry_count;
};

enum BmgDecodeResult bmg_decode(struct BmgFile *out, void *file);
void bmg_free_file(struct BmgFile *bmg);

struct BmgInfEntry bmg_get_inf_entry(struct BmgFile *bmg, uint16_t idx);
void bmg_get_dat_entry(struct BmgFile *file, size_t idx, char *res, size_t res_len);
uint64_t bmg_get_flw_instruction(struct BmgFile *bmg, uint16_t idx);
uint16_t bmg_get_flw_label(struct BmgFile *bmg, uint16_t idx);
uint8_t bmg_get_flw_id(struct BmgFile *bmg, uint16_t idx);
struct BmgFliEntry bmg_get_fli_entry(struct BmgFile *bmg, uint16_t idx);

void bmg_put_dat_entry(struct Vec *dest, const char *src, size_t src_len);
