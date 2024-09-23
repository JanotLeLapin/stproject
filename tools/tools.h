typedef typeof(sizeof(0)) size_t;

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
  unsigned int index;
  unsigned int attributes;
};

struct BmgFliEntry {
  unsigned int id;
  unsigned short index;
};

struct BmgFile {
  char *buf;
  size_t inf_offset, dat_offset, flw_instructions_offset, flw_labels_offset, fli_offset;
  unsigned short inf_entry_count, flw_instruction_count, flw_label_count, fli_entry_count;
};

enum BmgDecodeResult bmg_decode(struct BmgFile *out, void *file);
void bmg_free_file(struct BmgFile *bmg);

struct BmgInfEntry bmg_get_inf_entry(struct BmgFile *bmg, unsigned short idx);
unsigned long bmg_get_flw_instruction(struct BmgFile *bmg, unsigned short idx);
unsigned short bmg_get_flw_label(struct BmgFile *bmg, unsigned short idx);
unsigned char bmg_get_flw_id(struct BmgFile *bmg, unsigned short idx);
struct BmgFliEntry bmg_get_fli_entry(struct BmgFile *bmg, unsigned short idx);
