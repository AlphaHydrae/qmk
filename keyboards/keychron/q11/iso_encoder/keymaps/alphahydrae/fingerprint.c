#include "fingerprint.h"

#define FNV_PRIME 16777619u

uint32_t fingerprint_keycodes(uint32_t hash, const uint16_t *keycodes, size_t count) {
    // Each step is a bijection of the hash (XOR, then multiplying by an odd
    // number modulo 2^32). So two keymaps that differ in a single keycode
    // diverge at that step and never meet again.
    for (size_t i = 0; i < count; i++) {
        hash ^= keycodes[i];
        hash *= FNV_PRIME;
    }
    return hash;
}
