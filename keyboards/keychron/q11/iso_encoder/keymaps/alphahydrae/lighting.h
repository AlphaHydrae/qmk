#pragma once

// The color logic, as plain C, so that it can be unit-tested. Its only QMK
// dependency is the keycode values (quantum/keycodes.h, a standalone header).
// The QMK glue (keymap.c, rgb_matrix_user.inc) only reads the keyboard's state
// and applies the colors returned here.

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} lighting_rgb_t;

/**
 * Scales a color channel (0-255) by the backlight brightness (0-255, as
 * returned by rgb_matrix_get_val()).
 */
uint8_t lighting_scale(uint8_t value, uint8_t brightness);

/**
 * The per-layer base color, scaled by the brightness: blue on BASE and FN,
 * purple on BRACKETS and BRACKETS_FN. Takes the highest active layer.
 */
lighting_rgb_t base_color(uint8_t layer, uint8_t brightness);

/**
 * Whether the layer is one of the Fn layers (FN or BRACKETS_FN).
 */
bool is_fn_layer(uint8_t layer);

/**
 * The Fn color (dark red), scaled by the brightness.
 */
lighting_rgb_t fn_color(uint8_t brightness);

/**
 * Whether Caps Lock changes what the key types: KC_A to KC_Z. With the Mac's
 * input source set to Swiss French, Caps Lock leaves the accented keys alone.
 */
bool is_caps_affected(uint16_t keycode);

/**
 * Whether the key turns amber while Caps Lock is on: the keys Caps Lock
 * affects, and the Caps Lock key itself.
 */
bool is_caps_highlighted(uint16_t keycode);

/**
 * The color drawn on top of the current effect for one key, given the highest
 * active layer, the key's keycode on the layer in effect for it, the Caps Lock
 * state and the brightness.
 *
 * Returns true and sets *color when the key gets an overlay color: dark red on
 * the Fn layers, otherwise amber for the keys Caps Lock affects and the Caps
 * Lock key itself while it's on. Returns false (and leaves *color alone) when
 * the key keeps the color the effect drew.
 */
bool key_color(uint8_t layer, uint16_t keycode, bool caps_lock, uint8_t brightness, lighting_rgb_t *color);
