#if !defined(_REXFILE_)
#define _REXFILE_

/* REXFILE - for object files using the REX 36-bit format */

typedef struct {
    /* index into symbol table, 36 bits */
    unsigned long sym_idx;

    /*
     * word offset into section in least significant 18 bits
     * type in remaining bits, in practice 3 bits leaving most significant 15
     * bits reserved
     *
     *       Type    |Fill bits |Value
     *      ---------+----------+------------------
     *       ADDR18  | 18-35    | S+A
     *       ADDR27  | 9-35     | S+A
     *       PCRE18  | 18-35    | S+A-P
     *       PCRE27  | 9-35     | S+A-P
     *       ADDR18H | 19-35    | (S+A) >> 17
     *       ADDR18L | 19-35    | (S+A) & 0377777
     *       CPTR18S | 8-35     | ((S+A) << 1) + i
     *       CPTR18C | 7-35     | ((S+A) << 2) + i
     *      ---------+----------+------------------
     *      Value: S = symbol address, A = addend, P = load address
     *      Bits from most to least significant
     */
    unsigned int type_offset;
} rex_reloc_t;

#define REX_REL_TYPE(x) (((x).type_offset) >> 18)
#define REX_REL_OFFSET(x) (((x).type_offset) & 0777777)
#define REX_REL_MAKE_TYPE_OFFSET(x, y) (((unsigned int)(x) << 18) | (y))

typedef struct {
    /*
     * symbol name, should be packed into 10 7-bit characters in 2 36-bit
     * words
     */
    char name[10];

    /*
     * index into section table (18 bits), offset of symbol (18 bits) packed
     * into one 36-bit word
     * only valid if symbol is defined
     */
    unsigned long idx_offset;

    /* set if symbol is undefined, least significant bit of 36-bit word */
    int is_undef;
} rex_symbol_t;

#define REX_SYM_SECTION(x) (((x).idx_off) >> 18)
#define REX_SYM_OFFSET(x) (((x).idx_off) & 0777777)
#define REX_SYM_MAKE_SECTION_OFFSET(x, y) (((unsigned long)(x) << 18) | (y))

typedef struct {
    /*
     * section name, should be packed into 10 7-bit characters in 2 36-bit
     * words
     */
    char name[10];

    /*
     * load address in least significant 27 bits
     * type in remaining bits, in practice 2 bits leaving most significant 7
     * bits reserved
     *
     * Types: 0 = TEXT, 1 = DATA, 2 = BSS, 3 = reserved (maybe RODATA someday)
     */
    unsigned int type_load_addr;

    /* 36-bit word offset from start of file to raw data */
    unsigned long data_offset;

    /* Maximum section offset, least significant 18 bits of word */
    unsigned int data_size;

    /*
     * Index into relocation table of first relocation for this section
     * Set to 0 if no relocations
     */
    unsigned long rel_offset;

    /* Number of relocations for this section minus one */
    unsigned long rel_count;

    /* Reserve one more word for a total size of 8 words */
} rex_section_t;

#define REX_SECT_TYPE(x) (((x).type_load_addr) >> 27)
#define REX_SECT_LOAD_ADDR(x) (((x).type_load_addr) & 0777777777)
#define REX_SECT_MAKE_TYPE_LOAD_ADDR(x, y) (((unsigned long)(x) << 27) | (y))

typedef struct {
    /* must be 0xBADC0FFEE */
    unsigned long magic;

    /*
     * Two reserved bits; has relocation data (1 bit), has unresolved
     * references (1 bit), timestamp (32 bits, have fun in 2038)
     */
    unsigned long flags_time;

    /* Maximum section index, least significant 18 bits of word */
    unsigned int sections;

    /* Number of symbols in symbol table */
    unsigned long symbols;

    /* Number of relocation entries in relocation table */
    unsigned long relocs;

    /* Reserve two more words for a total size of 8 words */
} rex_header_t;

#define REX_HDR_MAGIC 0xBADC0FFEE

#define REX_HDR_FLAG_UR 1
#define REX_HDR_FLAG_RE 2

#define REX_HDR_FLAGS(x) (((x).flags_time) >> 32)
#define REX_HDR_TIME(x) (((x).flags_time) & 0xFFFFFFFF)
#define REX_HDR_MAKE_FLAGS_TIME(x, y) (((unsigned long)(x) << 32) | (y))

#endif
