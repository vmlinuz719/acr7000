#include "file36.h"
#include <stdlib.h>
#include <string.h>

static void pack29(uint64_t a, uint64_t b, uint8_t out[9]) {
    a &= 0xFFFFFFFFF;
    b &= 0xFFFFFFFFF;
    
    out[0] = a >> 28;
    out[1] = a >> 20;
    out[2] = a >> 12;
    out[3] = a >> 4;
    out[4] = a << 4 | b >> 32;
    out[5] = b >> 24;
    out[6] = b >> 16;
    out[7] = b >> 8;
    out[8] = b;
}

static void unpack92(uint8_t in[9], uint64_t *out_a, uint64_t *out_b) {
    *out_a = (
        (((uint64_t) in[0]) << 28) |
        (((uint64_t) in[1]) << 20) |
        (((uint64_t) in[2]) << 12) |
        (((uint64_t) in[3]) << 4) |
        (((uint64_t) in[4]) >> 4)
    ) & 0xFFFFFFFFF;
    
    *out_b = (
        (((uint64_t) in[4]) << 32) |
        (((uint64_t) in[5]) << 24) |
        (((uint64_t) in[6]) << 16) |
        (((uint64_t) in[7]) << 8) |
        ((uint64_t) in[8])
    ) & 0xFFFFFFFFF;
}

int f36seek(file36_t *file, long offset, int whence) {
    if (file == NULL) return -1;
    
    long tgt = 0;
    switch (whence) {
        case SEEK_SET: {
            tgt = offset;
        } break;
        
        case SEEK_CUR: {
            tgt = file->word_idx + offset;
        } break;
        
        case SEEK_END: {
            tgt = file->word_file_size + offset;
        } break;
        
        default: return -1;
    }
    
    if (tgt < 0) return -1;
    
    file->word_idx = tgt;
    file->eof = 0;
    file->error = 0;
    return 0;
}

long f36tell(file36_t *file) {
    if (file == NULL) return -1;
    return file->word_idx;
}

long f36size(file36_t *file) {
    if (file == NULL) return -1;
    return file->word_file_size;
}

int f36eof(file36_t *file) {
    if (file == NULL) return -1;
    return file->eof;
}

int f36error(file36_t *file) {
    if (file == NULL) return -1;
    return file->error;
}

size_t f36read(uint64_t *buf, size_t count, file36_t *file) {
    if (buf == NULL || file == NULL || count == 0) return 0;
    
    file->error = 0;
    size_t words_read = 0;
    while (words_read < count) {
        long dword_idx = file->word_idx >> 1;
        int word_select = file->word_idx & 1;
        
        if (fseek(file->file, dword_idx * 9, SEEK_SET) != 0) {
            file->error = 1;
            break;
        }
        
        uint8_t dword[9];
        size_t blocks_read = fread(dword, 9 * sizeof(uint8_t), 1, file->file);
        if (blocks_read != 1) {
            if (ferror(file->file)) file->error = 1;
            if (feof(file->file)) file->eof = 1;
            break;
        }
        
        uint64_t a, b;
        unpack92(dword, &a, &b);
        
        if (word_select) {
            buf[words_read++] = b;
            file->word_idx++;
        } else {
            buf[words_read++] = a;
            file->word_idx++;
            if (words_read < count) {
                buf[words_read++] = b;
                file->word_idx++;
            }
        }
    }
    
    return words_read;
}

size_t f36write(const uint64_t *buf, size_t count, file36_t *file) {
    if (buf == NULL || file == NULL || count == 0) return 0;
    
    file->error = 0;
    size_t words_written = 0;
    while (words_written < count) {
        long dword_idx = file->word_idx >> 1;
        int word_select = file->word_idx & 1;
        
        uint8_t dword[9] = {0,0,0,0,0,0,0,0,0};
        int read_block = 0;
        if (fseek(file->file, dword_idx * 9, SEEK_SET) == 0) {
            if (fread(dword, 9 * sizeof(uint8_t), 1, file->file) == 1) {
                read_block = 1;
            }
        }
        
        uint64_t a = 0, b = 0;
        if (read_block) {
            unpack92(dword, &a, &b);
        }
        
        if (word_select) {
            b = buf[words_written++];
        } else {
            a = buf[words_written++];
            if (words_written < count && (file->word_idx + 1) / 2 == dword_idx) {
                b = buf[words_written++];
            }
        }
        
        pack29(a, b, dword);
        
        fseek(file->file, dword_idx * 9, SEEK_SET);
        if (fwrite(dword, 9 * sizeof(uint8_t), 1, file->file) != 1) {
            if (ferror(file->file)) file->error = 1;
            break;
        }
        
        file->word_idx += (!word_select && words_written < count) ? 2 : 1;
        if (file->word_idx > file->word_file_size) {
            file->word_file_size = file->word_idx;
        }
    }
    
    return words_written;
}

file36_t *f36open(const char *fname, const char *mode) {
    FILE *file = fopen(fname, mode);
    if (!file) return NULL;

    file36_t *file_ctx = malloc(sizeof(file36_t));
    if (file_ctx == NULL) {
        fclose(file);
        return NULL;
    }

    file_ctx->file = file;
    file_ctx->word_idx = 0;
    file_ctx->write = (strchr(mode, 'w') != NULL || strchr(mode, '+') != NULL);

    fseek(file, 0, SEEK_END);
    long file_bytes = ftell(file);
    file_ctx->word_file_size = (file_bytes / 9) * 2;

    fseek(file, 0, SEEK_SET);
    return file_ctx;
}

int f36close(file36_t *file) {
    if (!file) return -1;
    int result = fclose(file->file);
    free(file);
    return result;
}