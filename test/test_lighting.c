// Tests for the color logic in lighting.c. The input space is small, so every
// combination of layer, keycode, Caps Lock state and brightness is checked, not
// random samples.

#include "check.h"
#include "keycodes.h"
#include "layers.h"
#include "lighting.h"

#define NUM_LAYERS 4

static bool rgb_equal(lighting_rgb_t a, lighting_rgb_t b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static bool rgb_is_black(lighting_rgb_t c) {
    return c.r == 0 && c.g == 0 && c.b == 0;
}

static bool rgb_within(lighting_rgb_t c, uint8_t brightness) {
    return c.r <= brightness && c.g <= brightness && c.b <= brightness;
}

static lighting_rgb_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    lighting_rgb_t c = {r, g, b};
    return c;
}

static void test_scale(void) {
    for (int value = 0; value <= 255; value++) {
        for (int brightness = 0; brightness <= 255; brightness++) {
            uint8_t scaled = lighting_scale((uint8_t)value, (uint8_t)brightness);
            CHECK(scaled == value * brightness / 255, "value %d, brightness %d: got %d", value, brightness, scaled);
            CHECK(scaled <= brightness, "value %d, brightness %d: got %d", value, brightness, scaled);
        }
        CHECK(lighting_scale((uint8_t)value, 255) == value, "value %d at full brightness", value);
        CHECK(lighting_scale((uint8_t)value, 0) == 0, "value %d at brightness 0", value);
    }
}

static void test_base_color(void) {
    // The colors at full brightness, as documented in the README.
    CHECK(rgb_equal(base_color(BASE, 255), rgb(0, 64, 128)), "BASE is blue");
    CHECK(rgb_equal(base_color(FN, 255), rgb(0, 64, 128)), "FN sits on BASE: blue");
    CHECK(rgb_equal(base_color(BRACKETS, 255), rgb(64, 0, 64)), "BRACKETS is purple");
    CHECK(rgb_equal(base_color(BRACKETS_FN, 255), rgb(64, 0, 64)), "BRACKETS_FN sits on BRACKETS: purple");

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        lighting_rgb_t full = base_color((uint8_t)layer, 255);
        for (int brightness = 0; brightness <= 255; brightness++) {
            lighting_rgb_t c = base_color((uint8_t)layer, (uint8_t)brightness);
            CHECK(rgb_within(c, (uint8_t)brightness), "layer %d, brightness %d", layer, brightness);
            CHECK(c.r == lighting_scale(full.r, (uint8_t)brightness) && c.g == lighting_scale(full.g, (uint8_t)brightness) && c.b == lighting_scale(full.b, (uint8_t)brightness), "layer %d, brightness %d: not the scaled full color", layer, brightness);
        }
        CHECK(rgb_is_black(base_color((uint8_t)layer, 0)), "layer %d at brightness 0", layer);
    }
}

static void test_fn(void) {
    CHECK(!is_fn_layer(BASE), "BASE");
    CHECK(is_fn_layer(FN), "FN");
    CHECK(!is_fn_layer(BRACKETS), "BRACKETS");
    CHECK(is_fn_layer(BRACKETS_FN), "BRACKETS_FN");

    CHECK(rgb_equal(fn_color(255), rgb(139, 0, 0)), "Fn is dark red");
    CHECK(rgb_is_black(fn_color(0)), "Fn at brightness 0");
    for (int brightness = 0; brightness <= 255; brightness++) {
        lighting_rgb_t c = fn_color((uint8_t)brightness);
        CHECK(rgb_within(c, (uint8_t)brightness), "brightness %d", brightness);
        CHECK(c.r == lighting_scale(139, (uint8_t)brightness) && c.g == 0 && c.b == 0, "brightness %d", brightness);
    }
}

static void test_caps_affected(void) {
    // Every letter, in the order of the alphabet, not just the first and last.
    const uint16_t letters[] = {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M,
                                KC_N, KC_O, KC_P, KC_Q, KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X, KC_Y, KC_Z};
    int            affected  = 0;
    for (size_t i = 0; i < sizeof(letters) / sizeof(letters[0]); i++) {
        CHECK(is_caps_affected(letters[i]), "letter %zu", i);
    }
    for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
        affected += is_caps_affected((uint16_t)keycode);
    }
    CHECK(affected == 26, "%d keycodes affected, not just the 26 letters", affected);

    // Keys around the letters that Caps Lock leaves alone on the Swiss French
    // Mac layout. Letters with modifiers (S(KC_A)…) are covered by the count
    // above; their macros aren't in keycodes.h.
    const uint16_t unaffected[] = {KC_TRNS, KC_NO, KC_CAPS, KC_1, KC_0, KC_LBRC, KC_RBRC, KC_SCLN,
                                   KC_QUOT, KC_NUHS, KC_GRV, KC_COMM, KC_DOT, KC_SLSH, KC_SPC};
    for (size_t i = 0; i < sizeof(unaffected) / sizeof(unaffected[0]); i++) {
        CHECK(!is_caps_affected(unaffected[i]), "keycode 0x%04X", unaffected[i]);
    }

    // The highlighted keys are the affected ones plus the Caps Lock key.
    int highlighted = 0;
    for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
        bool h = is_caps_highlighted((uint16_t)keycode);
        highlighted += h;
        CHECK(h == (is_caps_affected((uint16_t)keycode) || keycode == KC_CAPS), "keycode 0x%04X", keycode);
    }
    CHECK(highlighted == 27, "%d keycodes highlighted, not the 26 letters and Caps Lock", highlighted);
}

static void test_key_color(void) {
    // A color key_color() never returns, to check that it leaves *color alone.
    const lighting_rgb_t untouched = rgb(1, 2, 3);

    lighting_rgb_t c = untouched;
    CHECK(key_color(BASE, KC_A, true, 255, &c) && rgb_equal(c, rgb(255, 140, 0)), "Caps Lock is amber");
    c = untouched;
    CHECK(key_color(BASE, KC_CAPS, true, 255, &c) && rgb_equal(c, rgb(255, 140, 0)), "Caps Lock key is amber");
    c = untouched;
    CHECK(!key_color(BASE, KC_CAPS, false, 255, &c), "Caps Lock key without Caps Lock");

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int caps = 0; caps <= 1; caps++) {
            for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
                bool affected = is_caps_highlighted((uint16_t)keycode);
                for (int brightness = 0; brightness <= 255; brightness++) {
                    c              = untouched;
                    bool overlaid  = key_color((uint8_t)layer, (uint16_t)keycode, caps, (uint8_t)brightness, &c);
                    bool expect_on = is_fn_layer((uint8_t)layer) || (caps && affected);

                    CHECK(overlaid == expect_on, "layer %d, caps %d, keycode 0x%04X, brightness %d", layer, caps, keycode, brightness);
                    if (!overlaid) {
                        // No overlay: the key keeps the effect's color (the
                        // base color with layer_color).
                        CHECK(rgb_equal(c, untouched), "layer %d, caps %d, keycode 0x%04X: color changed", layer, caps, keycode);
                        continue;
                    }

                    // The Fn color wins over Caps Lock.
                    lighting_rgb_t expected = is_fn_layer((uint8_t)layer)
                                                  ? fn_color((uint8_t)brightness)
                                                  : rgb(lighting_scale(255, (uint8_t)brightness), lighting_scale(140, (uint8_t)brightness), 0);
                    CHECK(rgb_equal(c, expected), "layer %d, caps %d, keycode 0x%04X, brightness %d", layer, caps, keycode, brightness);
                    CHECK(rgb_within(c, (uint8_t)brightness), "layer %d, caps %d, keycode 0x%04X, brightness %d", layer, caps, keycode, brightness);
                    CHECK(brightness != 0 || rgb_is_black(c), "layer %d, caps %d, keycode 0x%04X", layer, caps, keycode);
                }
            }
        }
    }
}

int main(void) {
    test_scale();
    test_base_color();
    test_fn();
    test_caps_affected();
    test_key_color();
    return check_result();
}
