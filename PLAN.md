# Plan: layer lighting, Caps Lock and Fn overlays, tests

Planned changes to the Keychron Q11 firmware described in
[README.md](./README.md).

- [Goals](#goals)
- [Decisions](#decisions)
- [Step 1: extract the color logic and test it](#step-1-extract-the-color-logic-and-test-it)
- [Step 2: Caps Lock overlay](#step-2-caps-lock-overlay)
- [Step 3: Fn overlay](#step-3-fn-overlay)
- [Step 4: update the README](#step-4-update-the-readme)
- [Migrating an install from the symlink setup](#migrating-an-install-from-the-symlink-setup)
- [To verify against the QMK source](#to-verify-against-the-qmk-source)

## Goals

1. With Caps Lock on, the keys it affects light up amber.
2. With Fn held, only the keys the Fn layer actually maps light up red. The
   others are dimmed instead of the whole board turning red.
3. The color logic is unit-tested on the computer, without a keyboard.

## Decisions

| Topic                          | Decision                                                                                   |
| :----------------------------- | :----------------------------------------------------------------------------------------- |
| Caps Lock color                | Amber, about `255, 140, 0`, scaled by brightness                                           |
| Keys affected by Caps Lock     | A–Z only. On macOS Swiss French, Caps Lock leaves è, é, à… unchanged                       |
| Fn layer, mapped keys          | Red (`139, 0, 0` as today), scaled by brightness                                           |
| Fn layer, fall-through keys    | The base color, dimmed. Start at 50% and adjust ("not too dimmed")                         |
| Fn layer, `KC_NO` keys         | Treated like fall-through keys (dimmed). They don't fall through, but they do nothing      |
| Caps Lock while Fn is held     | Fn overlay wins. The Caps Lock highlight comes back when Fn is released                    |

## Step 1: extract the color logic and test it

Split the code so that the decisions are plain C with no QMK dependencies,
and only a thin loop talks to QMK.

```text
keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/
  keymap.c            keymaps[], encoder_map[], the QMK hooks (thin)
  layers.h            the layer names
  rgb_matrix_user.inc the custom effect (thin)
  lighting.h/.c       pure color logic, no QMK headers except keycodes
  rules.mk            SRC += lighting.c
test/
  test_lighting.c     plain C tests, assert-based
  Makefile            builds and runs the tests with the host compiler
```

`lighting.c` exposes small functions that take everything as arguments:

- `base_color(layer, brightness)`: per-layer color, scaled.
- `is_caps_affected(keycode)`: `KC_A`–`KC_Z`.
- `is_fn_mapped(keycode)`: not `KC_TRNS` and not `KC_NO`.
- `key_color(...)`: the final color of one key, given the layer, whether Fn is
  held, the key's keycode on the Fn layer and on the active layer, the Caps
  Lock state and the brightness.

The glue in `keymap.c` loops over the keys, looks up their keycodes through
QMK and calls `key_color()`.

The input space is small, so the tests check **every combination** rather than
random samples. Properties to check:

- Brightness 0 gives black, and no channel ever exceeds the brightness.
- Caps Lock off never gives amber. Caps Lock on gives amber exactly for the
  affected keycodes, and only when Fn isn't held.
- With Fn held, mapped keys are red and the others are the base color, dimmed.
- Fall-through keys are dimmed but never black when brightness isn't 0
  (guards the "not too dimmed" intent against rounding).
- Without Fn or Caps Lock, every key has the base color.

Keycode values (`KC_A`, `KC_TRNS`…) come from QMK's `quantum/keycodes.h`,
which the tests include from the `qmk_firmware` checkout.

Add a CI job that runs the tests on every push, next to the firmware build in
`.github/workflows/build_binaries.yaml`.

Later, if tap-hold keys, combos or tap dance are added, QMK's own test
framework (`qmk_firmware/tests/`, GoogleTest) can simulate key sequences
through the real QMK pipeline.

## Step 2: Caps Lock overlay

The overlays (this step and the next) go in
`rgb_matrix_indicators_advanced_user()`. That hook runs after **any** effect, so
the Caps Lock and Fn highlights also work on top of a stock effect.

The overlays look up keycodes on each half, so both halves must use the same
`keymaps[]`. Don't leave VIA edits on the keyboard (they only reach the left
half).

- `host_keyboard_led_state().caps_lock` gives the Caps Lock state.
- The Q11's keyboard code (`rgb_matrix_indicators_advanced_kb()` in `q11.c`)
  paints the Caps Lock key's LED (index 23) white when Caps Lock is on, but
  only if `rgb_matrix_indicators_advanced_user()` returns `true`. Return
  `false` so it doesn't overwrite the amber.
- `config.h`: `#define SPLIT_LED_STATE_ENABLE`. Without it, only the left half
  knows about Caps Lock, and Y, U, H, J, N, M… wouldn't light up.
- Each key's keycode is looked up on the layer actually in effect for it
  (`layer_switch_get_layer()` then `keymap_key_to_keycode()`), so transparent
  keys resolve to the layer below.

Affected keycodes, with the Mac's input source set to **Swiss French**: only
`KC_A`–`KC_Z`.

Checked on the Mac: with Caps Lock on, the keys right of the letters still
type `è¨éà$,.-`, exactly as without it. macOS Caps Lock doesn't uppercase the
accented keys on this layout, so they aren't highlighted.

## Step 3: Fn overlay

While Fn is held, dimmed fall-through keys show the dimmed **base color**,
whatever effect is selected. The alternative (dimming whatever the effect drew)
needs reading back LED colors, which QMK doesn't really offer.

While layer 1 or 3 is active:

- A key whose keycode **on the Fn layer** is mapped (`is_fn_mapped()`) is red.
- Other keys get the base color of the layer below, dimmed (50% to start).
- The lookup uses the Fn layer only, not `layer_switch_get_layer()`, because
  the question is "does Fn do something here", not "what does this key do
  now".

Possible later tweak: give the held Fn key its own color.

## Step 4: update the README

- Remove the "Fn red covers the whole board" Known quirk.
- Describe the new lighting and the tests.

## Migrating an install from the symlink setup

A QMK install set up before the userspace layout symlinked the old keymap
folder into `~/qmk_firmware`. To migrate it:

- Delete the old symlink in
  `~/qmk_firmware/keyboards/keychron/q11/iso_encoder/keymaps/`, which points to
  a folder that no longer exists.
- Run `qmk config user.overlay_dir="$(realpath .)"` from this repository.
- Reset `user.keymap` to `alphahydrae` if it was set.

## To verify against the QMK source

Written from memory of QMK, not checked against the current source:

- `SPLIT_LED_STATE_ENABLE` is the right option for syncing Caps Lock.
- `quantum/keycodes.h` compiles on its own for the host tests.
- How the Keychron VIA definition lists custom effects (cosmetic only).
