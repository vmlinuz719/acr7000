#if !defined(_FILE36_)
#define _FILE36_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    FILE *file;
    long word_idx, word_file_size;
    int write;
} file36_t;

file36_t *f36open(const char *fname, const char *mode);
int f36close(file36_t *file);

size_t f36read(uint64_t *buf, size_t count, file36_t *file);
size_t f36write(const uint64_t *buf, size_t count, file36_t *file);
int f36seek(file36_t *file, long offset, int whence);
long f36tell(file36_t *file);

#endif
