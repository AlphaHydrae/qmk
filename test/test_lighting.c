// Tests for the color logic in lighting.c. The input space is small, so every
// combination of layer, keycode, Caps Lock state and brightness is checked, not
// random samples. Only one of the two keycodes key_color() takes matters on a
// given layer, so the other one is sampled.

#include "check.h"
#include "keycodes.h"
#include "layers.h"
#include "lighting.h"

#define NUM_LAYERS 4

// MO() is in quantum_keycodes.h, which the tests don't include.
#define MO_FN (QK_MOMENTARY | FN)
#define MO_BRACKETS_FN (QK_MOMENTARY | BRACKETS_FN)

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

static void test_dimmed_base_color(void) {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        lighting_rgb_t full = base_color((uint8_t)layer, 255);
        for (int brightness = 0; brightness <= 255; brightness++) {
            lighting_rgb_t base   = base_color((uint8_t)layer, (uint8_t)brightness);
            lighting_rgb_t dimmed = dimmed_base_color((uint8_t)layer, (uint8_t)brightness);
            const uint8_t  f[]    = {full.r, full.g, full.b};
            const uint8_t  b[]    = {base.r, base.g, base.b};
            const uint8_t  d[]    = {dimmed.r, dimmed.g, dimmed.b};
            CHECK(rgb_within(dimmed, (uint8_t)brightness), "layer %d, brightness %d", layer, brightness);
            for (int i = 0; i < 3; i++) {
                if (f[i] == 0) {
                    CHECK(d[i] == 0, "layer %d, brightness %d, channel %d: not in the base color", layer, brightness, i);
                    continue;
                }
                if (brightness == 0) {
                    CHECK(d[i] == 0, "layer %d, channel %d: brightness 0 isn't black", layer, i);
                    continue;
                }
                // Dimmed, but never black ("not too dimmed", whatever the
                // rounding).
                CHECK(d[i] >= 1, "layer %d, brightness %d, channel %d: black", layer, brightness, i);
                // About half the base color: base / 2 <= dimmed <= base / 2 + 2.
                // At the lowest brightness levels, where the base color rounds
                // down to 0, this still allows 1.
                CHECK(b[i] <= 2 * d[i] && 2 * d[i] <= b[i] + 4, "layer %d, brightness %d, channel %d: %d is not about half of %d", layer, brightness, i, d[i], b[i]);
            }
            // Visibly dimmer than the base color, unless the brightness is so
            // low that the base color is (nearly) black anyway.
            if (brightness >= 16) {
                CHECK(dimmed.r + dimmed.g + dimmed.b < base.r + base.g + base.b, "layer %d, brightness %d: not dimmer", layer, brightness);
            }
        }
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

    // Every keycode is mapped except the transparent and the no-op ones.
    CHECK(!is_fn_mapped(KC_TRNS), "KC_TRNS");
    CHECK(!is_fn_mapped(KC_NO), "KC_NO");
    const uint16_t mapped[] = {KC_A, KC_DEL, KC_F1, KC_MUTE, KC_VOLU, KC_BRID, KC_MCTL, KC_LPAD, RM_TOGG, RM_NEXT, RM_VALU, NK_TOGG};
    for (size_t i = 0; i < sizeof(mapped) / sizeof(mapped[0]); i++) {
        CHECK(is_fn_mapped(mapped[i]), "keycode 0x%04X", mapped[i]);
    }
    int count = 0;
    for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
        count += is_fn_mapped((uint16_t)keycode);
    }
    CHECK(count == 0x10000 - 2, "%d keycodes mapped, not all but KC_TRNS and KC_NO", count);
}

static void test_layer_hold(void) {
    CHECK(is_layer_hold(MO_FN), "MO(FN)");
    CHECK(is_layer_hold(MO_BRACKETS_FN), "MO(BRACKETS_FN)");
    // Toggling a layer isn't holding it.
    CHECK(!is_layer_hold(QK_TOGGLE_LAYER | BRACKETS), "TG(BRACKETS)");
    CHECK(!is_layer_hold(KC_TRNS), "KC_TRNS");
    // Exactly the 32 MO() keycodes, one per possible layer.
    int count = 0;
    for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
        count += is_layer_hold((uint16_t)keycode);
    }
    CHECK(count == 32, "%d keycodes hold a layer, not the 32 MO() ones", count);
}

static void test_caps_affected(void) {
    // Every letter, in the order of the alphabet, not just the first and last.
    const uint16_t letters[] = {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O, KC_P, KC_Q, KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X, KC_Y, KC_Z};
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
    const uint16_t unaffected[] = {KC_TRNS, KC_NO, KC_CAPS, KC_1, KC_0, KC_LBRC, KC_RBRC, KC_SCLN, KC_QUOT, KC_NUHS, KC_GRV, KC_COMM, KC_DOT, KC_SLSH, KC_SPC};
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
    CHECK(key_color(BASE, KC_A, KC_NO, true, true, 255, &c) && rgb_equal(c, rgb(255, 140, 0)), "Caps Lock is amber");
    c = untouched;
    CHECK(key_color(BASE, KC_CAPS, KC_NO, true, true, 255, &c) && rgb_equal(c, rgb(255, 140, 0)), "Caps Lock key is amber");
    c = untouched;
    CHECK(!key_color(BASE, KC_CAPS, KC_NO, true, false, 255, &c), "Caps Lock key without Caps Lock");
    c = untouched;
    CHECK(key_color(FN, KC_Q, RM_NEXT, true, true, 255, &c) && rgb_equal(c, rgb(139, 0, 0)), "Fn+Q is red, even with Caps Lock");
    c = untouched;
    CHECK(key_color(FN, KC_Y, KC_TRNS, true, true, 255, &c) && rgb_equal(c, dimmed_base_color(FN, 255)), "Fn+Y falls through: dimmed blue, even with Caps Lock");
    c = untouched;
    CHECK(key_color(BRACKETS_FN, KC_NO, KC_NO, true, false, 255, &c) && rgb_equal(c, dimmed_base_color(BRACKETS, 255)), "Fn on an unused key: dimmed purple");
    c = untouched;
    CHECK(key_color(FN, KC_Q, RM_NEXT, false, false, 255, &c) && rgb_equal(c, rgb(139, 0, 0)), "Fn+Q over a stock effect: red");
    c = untouched;
    CHECK(key_color(FN, KC_Y, KC_TRNS, false, true, 255, &c) && rgb_is_black(c), "Fn+Y over a stock effect: black, even with Caps Lock");
    c = untouched;
    CHECK(key_color(BASE, KC_A, KC_NO, false, true, 255, &c) && rgb_equal(c, rgb(255, 140, 0)), "Caps Lock over a stock effect: amber");
    c = untouched;
    CHECK(key_color(FN, MO_FN, KC_TRNS, true, false, 255, &c) && rgb_equal(c, rgb(139, 0, 0)), "the held Fn key is red");
    c = untouched;
    CHECK(key_color(BRACKETS_FN, MO_BRACKETS_FN, KC_TRNS, false, false, 255, &c) && rgb_equal(c, rgb(139, 0, 0)), "the held Fn key is red over a stock effect");
    c = untouched;
    CHECK(!key_color(BASE, MO_FN, KC_NO, true, false, 255, &c), "the Fn key without Fn held keeps the effect's color");

    // Outside the Fn layers, every keycode, with a few Fn keycodes and both
    // effect states to check that they're ignored there.
    const uint16_t fn_keycodes[] = {KC_TRNS, KC_NO, RM_NEXT};
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        if (is_fn_layer((uint8_t)layer)) {
            continue;
        }
        for (size_t i = 0; i < sizeof(fn_keycodes) / sizeof(fn_keycodes[0]); i++) {
            for (int effect = 0; effect <= 1; effect++) {
                for (int caps = 0; caps <= 1; caps++) {
                    for (int keycode = 0; keycode <= 0xFFFF; keycode++) {
                        bool highlighted = caps && is_caps_highlighted((uint16_t)keycode);
                        for (int brightness = 0; brightness <= 255; brightness++) {
                            c             = untouched;
                            bool overlaid = key_color((uint8_t)layer, (uint16_t)keycode, fn_keycodes[i], effect, caps, (uint8_t)brightness, &c);

                            CHECK(overlaid == highlighted, "layer %d, caps %d, keycode 0x%04X, brightness %d", layer, caps, keycode, brightness);
                            if (!overlaid) {
                                // No overlay: the key keeps the effect's color
                                // (the base color with layer_color).
                                CHECK(rgb_equal(c, untouched), "layer %d, caps %d, keycode 0x%04X: color changed", layer, caps, keycode);
                                continue;
                            }
                            lighting_rgb_t amber = rgb(lighting_scale(255, (uint8_t)brightness), lighting_scale(140, (uint8_t)brightness), 0);
                            CHECK(rgb_equal(c, amber), "layer %d, keycode 0x%04X, brightness %d: not amber", layer, keycode, brightness);
                        }
                    }
                }
            }
        }
    }

    // On the Fn layers, every Fn keycode, with a few keycodes in effect (a
    // letter, the Caps Lock key, a key Caps Lock ignores) to check that Caps
    // Lock doesn't show through, and the Fn keys, which are always red.
    const uint16_t keycodes[] = {KC_A, KC_CAPS, KC_1, MO_FN, MO_BRACKETS_FN};
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        if (!is_fn_layer((uint8_t)layer)) {
            continue;
        }
        for (size_t i = 0; i < sizeof(keycodes) / sizeof(keycodes[0]); i++) {
            for (int effect = 0; effect <= 1; effect++) {
                for (int caps = 0; caps <= 1; caps++) {
                    for (int fn_keycode = 0; fn_keycode <= 0xFFFF; fn_keycode++) {
                        bool red = is_fn_mapped((uint16_t)fn_keycode) || is_layer_hold(keycodes[i]);
                        for (int brightness = 0; brightness <= 255; brightness++) {
                            c             = untouched;
                            bool overlaid = key_color((uint8_t)layer, keycodes[i], (uint16_t)fn_keycode, effect, caps, (uint8_t)brightness, &c);

                            // Every key gets the overlay: red, or for the others
                            // dimmed with layer_color and black otherwise.
                            CHECK(overlaid, "layer %d, effect %d, caps %d, Fn keycode 0x%04X, brightness %d", layer, effect, caps, fn_keycode, brightness);
                            lighting_rgb_t expected = red ? fn_color((uint8_t)brightness) : effect ? dimmed_base_color((uint8_t)layer, (uint8_t)brightness) : rgb(0, 0, 0);
                            CHECK(rgb_equal(c, expected), "layer %d, effect %d, caps %d, keycode 0x%04X, Fn keycode 0x%04X, brightness %d", layer, effect, caps, keycodes[i], fn_keycode, brightness);
                            CHECK(rgb_within(c, (uint8_t)brightness), "layer %d, Fn keycode 0x%04X, brightness %d", layer, fn_keycode, brightness);
                            // With layer_color, the dimmed keys are black exactly
                            // at brightness 0. The red ones round down to black
                            // at the lowest levels, like every full color.
                            CHECK(red || !effect || (brightness == 0) == rgb_is_black(c), "layer %d, Fn keycode 0x%04X, brightness %d: black exactly at brightness 0", layer, fn_keycode, brightness);
                        }
                    }
                }
            }
        }
    }
}

int main(void) {
    test_scale();
    test_base_color();
    test_dimmed_base_color();
    test_fn();
    test_layer_hold();
    test_caps_affected();
    test_key_color();
    return check_result();
}
