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
 * The base color, dimmed (about 50%) and scaled by the brightness: the color of
 * the keys that the Fn layer doesn't map while Fn is held. Rounds up, so it's
 * never black while the brightness isn't 0.
 */
lighting_rgb_t dimmed_base_color(uint8_t layer, uint8_t brightness);

/**
 * Whether the layer is one of the Fn layers (FN or BRACKETS_FN).
 */
bool is_fn_layer(uint8_t layer);

/**
 * The Fn color (dark red), scaled by the brightness.
 */
lighting_rgb_t fn_color(uint8_t brightness);

/**
 * Whether a keycode of the Fn layer does something: neither KC_TRNS (falls
 * through to the layer below) nor KC_NO (does nothing).
 */
bool is_fn_mapped(uint16_t keycode);

/**
 * Whether the keycode holds a layer on while pressed: MO(), which is how the
 * Fn keys activate the Fn layers.
 */
bool is_layer_hold(uint16_t keycode);

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
 * active layer, the key's keycode on the layer in effect for it, its keycode
 * on that highest layer (only used on the Fn layers), whether the current
 * effect is layer_color (only used on the Fn layers), the Caps Lock state and
 * the brightness.
 *
 * Returns true and sets *color when the key gets an overlay color. On the Fn
 * layers, every key does: dark red if the Fn layer maps it (is_fn_mapped()) or
 * if it's an Fn key (is_layer_hold() on its keycode in effect), otherwise the dimmed base color with the layer_color effect, and black with
 * any other effect. Elsewhere, the keys Caps Lock affects and
 * the Caps Lock key itself are amber while it's on. Returns false (and leaves
 * *color alone) when the key keeps the color the effect drew.
 */
bool key_color(uint8_t layer, uint16_t keycode, uint16_t fn_keycode, bool layer_effect, bool caps_lock, uint8_t brightness, lighting_rgb_t *color);
