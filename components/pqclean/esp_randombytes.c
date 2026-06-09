#include "randombytes.h"

#include "esp_random.h"
#include "esp_system.h"
#include <stddef.h>

int PQCLEAN_randombytes(uint8_t *output, size_t n) {
    if (n == 0 || output == NULL) {
        return 0;
    }
    esp_fill_random(output, n);
    return 0;
}
