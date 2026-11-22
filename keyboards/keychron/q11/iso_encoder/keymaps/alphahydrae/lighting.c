#include "lighting.h"

#include "keycodes.h"
#include "layers.h"

static const lighting_rgb_t BLUE     = {0, 64, 128};
static const lighting_rgb_t PURPLE   = {64, 0, 64};
static const lighting_rgb_t DARK_RED = {139, 0, 0};
static const lighting_rgb_t AMBER    = {255, 140, 0};

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

lighting_rgb_t base_color(uint8_t layer, uint8_t brightness) {
    bool brackets = layer == BRACKETS || layer == BRACKETS_FN;
    return scale_rgb(brackets ? PURPLE : BLUE, brightness);
}

bool is_fn_layer(uint8_t layer) {
    return layer == FN || layer == BRACKETS_FN;
}

lighting_rgb_t fn_color(uint8_t brightness) {
    return scale_rgb(DARK_RED, brightness);
}

bool is_caps_affected(uint16_t keycode) {
    return keycode >= KC_A && keycode <= KC_Z;
}

bool is_caps_highlighted(uint16_t keycode) {
    return is_caps_affected(keycode) || keycode == KC_CAPS;
}

bool key_color(uint8_t layer, uint16_t keycode, bool caps_lock, uint8_t brightness, lighting_rgb_t *color) {
    // The Fn color wins over Caps Lock, which comes back when Fn is released.
    if (is_fn_layer(layer)) {
        *color = fn_color(brightness);
        return true;
    }
    if (caps_lock && is_caps_highlighted(keycode)) {
        *color = scale_rgb(AMBER, brightness);
        return true;
    }
    return false;
}
