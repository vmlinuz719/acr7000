// Using a 36-bit disk image

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BLKSIZE_W 512
#define BLKSIZE_B ((BLKSIZE_W / 2) * 9)

// multiply by 2 ^ capacity code (315-10: 0; 15: 1; 20: 2)
#define IMGSIZE_315_10 18874368
#define LBASIZE_315_10 8192

static inline uint64_t load_word(char *data, int index) {
    int dword_index = (index / 2) * 9;
    
    if (!(index & 1)) {
        return (
            (((uint64_t) data[dword_index]) << 28) |
            (((uint64_t) data[dword_index + 1]) << 20) |
            (((uint64_t) data[dword_index + 2]) << 12) |
            (((uint64_t) data[dword_index + 3]) << 4) |
            (((uint64_t) data[dword_index + 4]) >> 4)
        );
    } else {
        return (
            (((uint64_t) (data[dword_index + 4] & 0xF)) << 32) |
            (((uint64_t) data[dword_index + 5]) << 24) |
            (((uint64_t) data[dword_index + 6]) << 16) |
            (((uint64_t) data[dword_index + 7]) << 8) |
            ((uint64_t) data[dword_index + 8])
        );
    }
}

static inline void store_word(char *data, int index, uint64_t value) {
    int dword_index = (index / 2) * 9;
    
    if (!(index & 1)) {
        data[dword_index] = value >> 28;
        data[dword_index + 1] = value >> 20;
        data[dword_index + 2] = value >> 12;
        data[dword_index + 3] = value >> 4;
        data[dword_index + 4] = (value << 4) | (data[dword_index + 4] & 0xF);
    } else {
        data[dword_index + 4] = (value >> 32) | (data[dword_index + 4] & 0xF0);
        data[dword_index + 5] = value >> 24;
        data[dword_index + 6] = value >> 16;
        data[dword_index + 7] = value >> 8;
        data[dword_index + 8] = value;
    }
}

typedef struct {
    FILE *file;
    
    int lba, capacity;
} disk_t;

int disk_seek(disk_t *disk, int lba) {
    if (lba < 0 || lba >= (LBASIZE_315_10 << disk->capacity)) {
        return -1;
    }
    
    disk->lba = lba;
    return 0;
}

int disk_read(disk_t *disk, char *buf) {
    if (disk->lba < 0 || disk->lba >= (LBASIZE_315_10 << disk->capacity)) {
        return -1;
    }
    
    fseek(disk->file, (disk->lba++) * BLKSIZE_B, SEEK_SET);
    
    int success = fread(buf, BLKSIZE_B, 1, disk->file);
    if (success != 1) {
        return -1;
    }
    
    return 0;
}

int disk_write(disk_t *disk, char *buf) {
    if (disk->lba < 0 || disk->lba >= (LBASIZE_315_10 << disk->capacity)) {
        return -1;
    }
    
    fseek(disk->file, (disk->lba++) * BLKSIZE_B, SEEK_SET);
    
    int success = fwrite(buf, BLKSIZE_B, 1, disk->file);
    if (success != 1) {
        return -1;
    }
    
    return 0;
}

