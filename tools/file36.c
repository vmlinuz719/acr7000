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

