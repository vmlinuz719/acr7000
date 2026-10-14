#include "rexfile.h"
#include "file36.h"

#include <stdio.h>
#include <time.h>

int load_hdr(file36_t *file, rex_header_t *hdr) {
    uint64_t header_buf[8];
    int read_words = f36read(header_buf, 8, file);
    if (read_words != 8) {
        return -1;
    }

    if (header_buf[0] != REX_HDR_MAGIC) {
        return -1;
    }

    hdr->magic = header_buf[0];
    hdr->flags_time = header_buf[1] & 0x3FFFFFFFF;
    hdr->sections = header_buf[2] & 0777777;
    hdr->symbols = header_buf[3];
    hdr->relocs = header_buf[4];
    return 0;
}

int save_hdr(file36_t *file, rex_header_t *hdr) {
    uint64_t header_buf[8] = {0,0,0,0,0,0,0,0};

    header_buf[0] = REX_HDR_MAGIC;
    header_buf[1] = hdr->flags_time;
    header_buf[2] = hdr->sections & 0777777;
    header_buf[3] = hdr->symbols;
    header_buf[4] = hdr->relocs;

    int result = f36write(header_buf, 8, file);
    return -1 * (result != 8);
}

void print_hdr(rex_header_t *hdr) {
    printf("=== REX File Header ===\n");
    printf("  Magic:\t0x%09lX\n", hdr->magic);
    printf(
        "  Flags:\t%s%s%s\n",
        (REX_HDR_FLAGS(*hdr) & REX_HDR_FLAG_RE) ? "RELOC" : "",
        (REX_HDR_FLAGS(*hdr) == (REX_HDR_FLAG_RE | REX_HDR_FLAG_UR)) ? "," : "",
        (REX_HDR_FLAGS(*hdr) & REX_HDR_FLAG_UR) ? "UNDEF" : ""
    );
    printf("  Sections:\t%u\n", hdr->sections);
    printf("  Symbols:\t%lu\n", hdr->symbols);
    printf("  Relocs:\t%lu\n", hdr->relocs);
}

int main(int argc, char *argv[]) {
    file36_t *file = f36open("test_rex_file.dat", "wb+");
    rex_header_t hdr = {
        .magic = REX_HDR_MAGIC,
        .flags_time = REX_HDR_MAKE_FLAGS_TIME(REX_HDR_FLAG_RE | REX_HDR_FLAG_UR, (int)time(NULL)),
        .sections = 64,
        .symbols = 12345,
        .relocs = 20472
    };
    f36seek(file, 0, SEEK_SET);
    save_hdr(file, &hdr);
    f36close(file);

    printf("Wrote fresh header to file...\n");

    file36_t *file2 = f36open("test_rex_file.dat", "rb");
    rex_header_t hdr2;
    int result = load_hdr(file2, &hdr2);
    print_hdr(&hdr2);
    f36close(file2);

    return 0;
}
