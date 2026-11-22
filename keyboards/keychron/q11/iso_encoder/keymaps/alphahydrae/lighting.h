#pragma once

// The color logic, as plain C with no QMK dependencies, so that it can be
// unit-tested. The QMK glue (keymap.c, rgb_matrix_user.inc) only reads the
// keyboard's state and applies the colors returned here.

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
