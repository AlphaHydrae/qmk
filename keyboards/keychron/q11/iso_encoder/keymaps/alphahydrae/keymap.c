/* Copyright 2023 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#include "fingerprint.h"
#include "layers.h"
#include "lighting.h"

#ifdef DYNAMIC_KEYMAP_ENABLE
#    include "dynamic_keymap.h"
#endif

enum custom_keycodes {
    // Switches the backlight to the layer_color effect (and turns it on).
    LYR_RGB = SAFE_RANGE,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_92_iso(
        KC_MUTE,       KC_ESC,   KC_F1,    KC_F2,    KC_F3,            KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_F11,           KC_F12,   KC_INS,   KC_DEL,          KC_MUTE,
        TG(BRACKETS),  KC_NUBS,  KC_1,     KC_2,     KC_3,             KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,       KC_MINS,          KC_EQL,   KC_BSPC,                   KC_PGUP,
        XXXXXXX,       KC_TAB,   KC_Q,     KC_W,     KC_E,             KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,       KC_LBRC,          KC_RBRC,                             KC_PGDN,
        XXXXXXX,       KC_CAPS,  KC_A,     KC_S,     KC_D,             KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,    KC_QUOT,          KC_NUHS,  KC_ENT,                    KC_HOME,
        XXXXXXX,       KC_LSFT,  KC_GRV,   KC_Z,     KC_X,             KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,     KC_SLSH,                    KC_RSFT,  KC_UP,
        LYR_RGB,       KC_LCTL,  KC_LOPT,  KC_LCMD,  MO(FN),                     KC_SPC,                       KC_SPC,             KC_RCMD,    MO(FN),           KC_RCTL,  KC_LEFT,  KC_DOWN,         KC_RGHT),

    [FN] = LAYOUT_92_iso(
        RM_TOGG,       _______,  KC_BRID,  KC_BRIU,  KC_MCTL,          KC_LPAD,  RM_VALD,  RM_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,    KC_VOLD,          KC_VOLU,  _______,  C(KC_KB_POWER),  RM_TOGG,
        _______,       _______,  _______,  _______,  _______,          _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,          _______,  KC_DEL,                    _______,
        _______,       RM_TOGG,  RM_NEXT,  RM_VALU,  RM_HUEU,          RM_SATU,  RM_SPDU,  _______,  _______,  _______,  _______,  _______,    _______,          _______,                             _______,
        _______,       _______,  RM_PREV,  RM_VALD,  RM_HUED,          RM_SATD,  RM_SPDD,  _______,  _______,  _______,  _______,  _______,    _______,          _______,  _______,                   _______,
        _______,       _______,  _______,  _______,  _______,          _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,    _______,                    _______,  _______,
        _______,       _______,  _______,  _______,  _______,                    _______,                      _______,            _______,    _______,          _______,  _______,  _______,         _______),

    [BRACKETS] = LAYOUT_92_iso(
        KC_MUTE,       KC_ESC,   KC_F1,    KC_F2,    KC_F3,            KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,     KC_F11,           KC_F12,   KC_INS,   KC_DEL,          KC_MUTE,
        TG(BRACKETS),  KC_NUBS,  KC_1,     KC_2,     KC_3,             KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,       KC_MINS,          KC_EQL,   KC_BSPC,                   KC_PGUP,
        XXXXXXX,       KC_TAB,   KC_Q,     KC_W,     KC_E,             KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,       A(KC_5),          A(KC_6),                             KC_PGDN,
        XXXXXXX,       KC_CAPS,  KC_A,     KC_S,     KC_D,             KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     LSA(KC_7),  A(KC_8),          A(KC_9),  KC_ENT,                    KC_HOME,
        XXXXXXX,       KC_LSFT,  KC_GRV,   KC_Z,     KC_X,             KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,     KC_SLSH,                    KC_RSFT,  KC_UP,
        LYR_RGB,       KC_LCTL,  KC_LOPT,  KC_LCMD,  MO(BRACKETS_FN),            KC_SPC,                       KC_SPC,             KC_RCMD,    MO(BRACKETS_FN),  KC_RCTL,  KC_LEFT,  KC_DOWN,         KC_RGHT),

    [BRACKETS_FN] = LAYOUT_92_iso(
        RM_TOGG,       _______,  KC_BRID,  KC_BRIU,  KC_MCTL,          KC_LPAD,  RM_VALD,  RM_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,    KC_VOLD,          KC_VOLU,  _______,  C(KC_KB_POWER),  RM_TOGG,
        _______,       _______,  _______,  _______,  _______,          _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,          _______,  KC_DEL,                    _______,
        _______,       RM_TOGG,  RM_NEXT,  RM_VALU,  RM_HUEU,          RM_SATU,  RM_SPDU,  _______,  _______,  _______,  _______,  _______,    _______,          _______,                             _______,
        _______,       _______,  RM_PREV,  RM_VALD,  RM_HUED,          RM_SATD,  RM_SPDD,  _______,  _______,  _______,  _______,  _______,    _______,          _______,  _______,                   _______,
        _______,       _______,  _______,  _______,  _______,          _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,    _______,                    _______,  _______,
        _______,       _______,  _______,  _______,  _______,                    _______,                      _______,            _______,    _______,          _______,  _______,  _______,         _______),
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [BASE]        = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [FN]          = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_VALD, RM_VALU) },
    [BRACKETS]    = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [BRACKETS_FN] = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_VALD, RM_VALU) },
};
#endif // ENCODER_MAP_ENABLE
// clang-format on

#ifdef DYNAMIC_KEYMAP_ENABLE
// VIA keeps its own copy of the keymap in the EEPROM, and QMK only reloads it
// from keymaps[] when the firmware was built on a different day. This reloads
// it whenever keymaps[] or encoder_map[] changed since the last startup, by
// comparing their fingerprint with the one saved in the EEPROM's user slot.
// keyboard_post_init_user() runs after QMK's own check (via_init()).
void keyboard_post_init_user(void) {
    uint32_t fingerprint = fingerprint_keycodes(FINGERPRINT_INIT, &keymaps[0][0][0], sizeof(keymaps) / sizeof(keymaps[0][0][0]));
#    ifdef ENCODER_MAP_ENABLE
    fingerprint = fingerprint_keycodes(fingerprint, &encoder_map[0][0][0], sizeof(encoder_map) / sizeof(encoder_map[0][0][0]));
#    endif // ENCODER_MAP_ENABLE

    if (eeconfig_read_user() != fingerprint) {
        dynamic_keymap_reset();
        eeconfig_update_user(fingerprint);
    }
}
#endif // DYNAMIC_KEYMAP_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LYR_RGB:
            if (record->event.pressed) {
                rgb_matrix_enable();
                rgb_matrix_mode(RGB_MATRIX_CUSTOM_layer_color);
            }
            return false;
    }
    return true;
}

// Runs after every RGB effect (the custom layer_color one or a stock one), for
// the LEDs in [led_min, led_max). Draws the Fn and Caps Lock overlays, decided
// per key by key_color() in lighting.c.
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer      = get_highest_layer(layer_state | default_layer_state);
    bool    caps_lock  = host_keyboard_led_state().caps_lock;
    uint8_t brightness = rgb_matrix_get_val();
    // Whether the custom per-layer color effect is on, rather than a stock one.
    bool layer_effect = rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_layer_color;

    // Every LED of the Q11 sits under a key, so looping over the matrix covers
    // them all. The matrix spans both halves; each half only draws its own LEDs.
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) {
                continue;
            }

            // The keycode on the layer in effect for this key, so that
            // transparent keys resolve to the layer below (for Caps Lock).
            keypos_t key     = {.col = col, .row = row};
            uint16_t keycode = keymap_key_to_keycode(layer_switch_get_layer(key), key);
            // The keycode on the Fn layer itself, without falling through:
            // the question is whether Fn does something for this key.
            uint16_t       fn_keycode = is_fn_layer(layer) ? keymap_key_to_keycode(layer, key) : KC_NO;
            lighting_rgb_t color;
            if (key_color(layer, keycode, fn_keycode, layer_effect, caps_lock, brightness, &color)) {
                rgb_matrix_set_color(led, color.r, color.g, color.b);
            }
        }
    }

    // false stops the keyboard code (q11.c) from painting the Caps Lock key
    // white on top.
    return false;
}
