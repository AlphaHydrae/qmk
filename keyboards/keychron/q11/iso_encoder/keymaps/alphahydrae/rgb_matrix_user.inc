// Custom RGB matrix effects, included by QMK's rgb_matrix.c (no #pragma once,
// see https://docs.qmk.fm/features/rgb_matrix#custom-rgb-matrix-effects).

// The per-layer base color: blue on layer 0, purple when the brackets layer is
// on (toggled, or the default layer on the Windows switch position).
RGB_MATRIX_EFFECT(layer_color)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "lighting.h"

static bool layer_color(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // The color itself is decided in lighting.c. On the Fn layers, this is the
    // color of the layer below, and the Fn color is drawn on top by
    // rgb_matrix_indicators_advanced_user() in keymap.c.
    uint8_t        layer = get_highest_layer(layer_state | default_layer_state);
    lighting_rgb_t color = base_color(layer, rgb_matrix_get_val());
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, color.r, color.g, color.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
