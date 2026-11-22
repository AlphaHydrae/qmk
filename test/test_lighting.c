// Tests for the color logic in lighting.c. The input space is small, so every
// combination of layer and brightness is checked, not random samples.

#include <stdio.h>
#include <stdlib.h>

#include "layers.h"
#include "lighting.h"

#define NUM_LAYERS 4

static int checks;
static int failures;

#define CHECK(cond, ...)                                       \
    do {                                                       \
        checks++;                                              \
        if (!(cond)) {                                         \
            failures++;                                        \
            fprintf(stderr, "%s:%d: %s: ", __FILE__, __LINE__, \
                    #cond);                                    \
            fprintf(stderr, __VA_ARGS__);                      \
            fputc('\n', stderr);                               \
        }                                                      \
    } while (0)

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

int main(void) {
    test_scale();
    test_base_color();
    test_fn();

    if (failures > 0) {
        fprintf(stderr, "%d of %d checks failed\n", failures, checks);
        return EXIT_FAILURE;
    }
    printf("All %d checks passed\n", checks);
    return EXIT_SUCCESS;
}
