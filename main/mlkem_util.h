#ifndef MLKEM_UTIL_H
#define MLKEM_UTIL_H

#include <stdint.h>
#include <stddef.h>

static inline uint32_t mlkem_cycle_diff(uint32_t start, uint32_t end) {
    return (uint32_t)(end - start);
}

static inline void mlkem_secure_zero(void *buf, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)buf;
    while (len--) {
        *p++ = 0;
    }
}

#endif
