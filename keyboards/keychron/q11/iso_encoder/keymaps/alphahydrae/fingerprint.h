#pragma once

// A fingerprint of the keymap compiled into the firmware, as plain C with no
// QMK dependencies, so that it can be unit-tested. keymap.c compares it with
// the one saved in the EEPROM at startup, and reloads VIA's copy of the keymap
// when they differ (see "How firmware and VIA interact" in the README).

#include <stddef.h>
#include <stdint.h>

/**
 * The starting value of a fingerprint (the 32-bit FNV-1a offset basis).
 */
#define FINGERPRINT_INIT 2166136261u

/**
 * Continues the fingerprint `hash` over `count` keycodes, one keycode at a
 * time (FNV-1a on 16-bit values). Start with FINGERPRINT_INIT, and chain calls
 * to cover several arrays.
 *
 * Changing any single keycode always changes the result.
 */
uint32_t fingerprint_keycodes(uint32_t hash, const uint16_t *keycodes, size_t count);
