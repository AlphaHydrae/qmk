// Tests for the keymap fingerprint in fingerprint.c. The property that matters
// is that changing any single keycode changes the fingerprint, so that a layout
// change always reloads VIA's copy of the keymap.

#include "check.h"
#include "fingerprint.h"

// About the size of keymaps[] plus encoder_map[] (4 layers).
#define KEYMAP_SIZE 512

// Arbitrary but fixed keycodes (a linear congruential generator), so failures
// are reproducible.
static void fill(uint16_t *keycodes, size_t count, uint32_t seed) {
    for (size_t i = 0; i < count; i++) {
        seed        = seed * 1103515245u + 12345u;
        keycodes[i] = (uint16_t)(seed >> 16);
    }
}

static int compare_u32(const void *a, const void *b) {
    uint32_t x = *(const uint32_t *)a;
    uint32_t y = *(const uint32_t *)b;
    return (x > y) - (x < y);
}

static void test_empty_and_chaining(void) {
    uint16_t keymap[KEYMAP_SIZE];
    fill(keymap, KEYMAP_SIZE, 1);

    CHECK(fingerprint_keycodes(FINGERPRINT_INIT, keymap, 0) == FINGERPRINT_INIT, "no keycodes");

    // Chaining calls (keymaps[], then encoder_map[]) is the same as one call.
    uint32_t whole = fingerprint_keycodes(FINGERPRINT_INIT, keymap, KEYMAP_SIZE);
    for (size_t split = 0; split <= KEYMAP_SIZE; split++) {
        uint32_t chained = fingerprint_keycodes(fingerprint_keycodes(FINGERPRINT_INIT, keymap, split), keymap + split, KEYMAP_SIZE - split);
        CHECK(chained == whole, "split at %zu", split);
    }
}

// One step never maps two different keycodes to the same hash, whatever the
// hash before it. This is why a single changed keycode is always detected.
static void test_step_is_injective(void) {
    static uint32_t outputs[0x10000];
    const uint32_t  before[] = {FINGERPRINT_INIT, 0, 1, 0xFFFFFFFFu, 0x12345678u, 0x80000000u};

    for (size_t b = 0; b < sizeof(before) / sizeof(before[0]); b++) {
        for (uint32_t keycode = 0; keycode <= 0xFFFF; keycode++) {
            uint16_t kc       = (uint16_t)keycode;
            outputs[keycode] = fingerprint_keycodes(before[b], &kc, 1);
        }
        qsort(outputs, 0x10000, sizeof(outputs[0]), compare_u32);
        int duplicates = 0;
        for (size_t i = 1; i < 0x10000; i++) {
            duplicates += outputs[i] == outputs[i - 1];
        }
        CHECK(duplicates == 0, "hash before 0x%08X: %d duplicates", before[b], duplicates);
    }
}

// End to end: every position of a keymap-sized array, changed to many other
// keycodes, changes the fingerprint.
static void test_single_change(void) {
    uint16_t keymap[KEYMAP_SIZE];
    fill(keymap, KEYMAP_SIZE, 2);
    uint32_t original = fingerprint_keycodes(FINGERPRINT_INIT, keymap, KEYMAP_SIZE);

    for (size_t i = 0; i < KEYMAP_SIZE; i++) {
        uint16_t saved = keymap[i];
        for (uint32_t delta = 1; delta <= 256; delta++) {
            // Small changes (neighboring keycodes) and changes of the high
            // byte (modifiers, layer keys).
            uint16_t replacements[] = {(uint16_t)(saved + delta), (uint16_t)(saved ^ (delta << 8))};
            for (size_t r = 0; r < 2; r++) {
                if (replacements[r] == saved) {
                    continue;
                }
                keymap[i] = replacements[r];
                CHECK(fingerprint_keycodes(FINGERPRINT_INIT, keymap, KEYMAP_SIZE) != original, "position %zu: 0x%04X -> 0x%04X", i, saved, replacements[r]);
            }
        }
        keymap[i] = saved;
    }
}

int main(void) {
    test_empty_and_chaining();
    test_step_is_injective();
    test_single_change();
    return check_result();
}
