#include "lighting.h"

#include "keycodes.h"
#include "layers.h"

static const lighting_rgb_t BLUE     = {0, 64, 128};
static const lighting_rgb_t PURPLE   = {64, 0, 64};
static const lighting_rgb_t DARK_RED = {139, 0, 0};
static const lighting_rgb_t AMBER    = {255, 140, 0};
static const lighting_rgb_t BLACK    = {0, 0, 0};

// How much the keys that the Fn layer doesn't map are dimmed, out of 255 (128
// is about 50%). Adjust to taste: the keys should stay clearly visible.
#define FN_DIM 128

static lighting_rgb_t scale_rgb(lighting_rgb_t color, uint8_t brightness) {
    lighting_rgb_t scaled = {
        lighting_scale(color.r, brightness),
        lighting_scale(color.g, brightness),
        lighting_scale(color.b, brightness),
    };
    return scaled;
}

uint8_t lighting_scale(uint8_t value, uint8_t brightness) {
    return (uint8_t)(((uint16_t)value * brightness) / 255);
}

// The base color at full brightness.
static lighting_rgb_t base_rgb(uint8_t layer) {
    bool brackets = layer == BRACKETS || layer == BRACKETS_FN;
    return brackets ? PURPLE : BLUE;
}

// Scales a channel by the brightness and by FN_DIM, rounding up so that a
// channel that isn't 0 never turns black while the brightness isn't 0.
static uint8_t dim(uint8_t value, uint8_t brightness) {
    uint32_t product = (uint32_t)value * brightness * FN_DIM;
    uint32_t divisor = 255UL * 255UL;
    return (uint8_t)((product + divisor - 1) / divisor);
}

lighting_rgb_t base_color(uint8_t layer, uint8_t brightness) {
    return scale_rgb(base_rgb(layer), brightness);
}

lighting_rgb_t dimmed_base_color(uint8_t layer, uint8_t brightness) {
    lighting_rgb_t full   = base_rgb(layer);
    lighting_rgb_t dimmed = {
        dim(full.r, brightness),
        dim(full.g, brightness),
        dim(full.b, brightness),
    };
    return dimmed;
}

bool is_fn_layer(uint8_t layer) {
    return layer == FN || layer == BRACKETS_FN;
}

lighting_rgb_t fn_color(uint8_t brightness) {
    return scale_rgb(DARK_RED, brightness);
}

bool is_fn_mapped(uint16_t keycode) {
    return keycode != KC_TRNS && keycode != KC_NO;
}

bool is_layer_hold(uint16_t keycode) {
    return IS_QK_MOMENTARY(keycode);
}

bool is_caps_affected(uint16_t keycode) {
    return keycode >= KC_A && keycode <= KC_Z;
}

bool is_caps_highlighted(uint16_t keycode) {
    return is_caps_affected(keycode) || keycode == KC_CAPS;
}

bool key_color(uint8_t layer, uint16_t keycode, uint16_t fn_keycode, bool layer_effect, bool caps_lock, uint8_t brightness, lighting_rgb_t *color) {
    // The Fn overlay wins over Caps Lock, which comes back when Fn is released.
    if (is_fn_layer(layer)) {
        // The held Fn key is transparent on the Fn layer, so it's recognized
        // by its keycode on the layer below (MO()).
        if (is_fn_mapped(fn_keycode) || is_layer_hold(keycode)) {
            *color = fn_color(brightness);
        } else {
            // The dimmed base color matches the layer_color effect. Over any
            // other effect, it would look out of place, so the key goes dark.
            *color = layer_effect ? dimmed_base_color(layer, brightness) : BLACK;
        }
        return true;
    }
    if (caps_lock && is_caps_highlighted(keycode)) {
        *color = scale_rgb(AMBER, brightness);
        return true;
    }
    return false;
}
