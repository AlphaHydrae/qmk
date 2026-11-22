// Custom RGB matrix effects, included by QMK's rgb_matrix.c (no #pragma once,
// see https://docs.qmk.fm/features/rgb_matrix#custom-rgb-matrix-effects).

// The per-layer base color: blue on layer 0, purple when the brackets layer is
// on (toggled, or the default layer on the Windows switch position).
RGB_MATRIX_EFFECT(layer_color)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "layers.h"

static bool layer_color(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t brightness = rgb_matrix_get_val();
#    define LAYER_COLOR_SCALE(val) ((uint8_t)(((uint16_t)(val) * brightness) / 255))

    // The Fn layers sit on top of a base layer, so ignore them here. The Fn
    // color is drawn by rgb_matrix_indicators_advanced_user() in keymap.c.
    bool brackets = layer_state_cmp(layer_state | default_layer_state, BRACKETS);

    uint8_t r = LAYER_COLOR_SCALE(brackets ? 64 : 0);
    uint8_t g = LAYER_COLOR_SCALE(brackets ? 0 : 64);
    uint8_t b = LAYER_COLOR_SCALE(brackets ? 64 : 128);
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
