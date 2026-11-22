# Plan: layer lighting, Caps Lock and Fn overlays, tests

Planned changes to the Keychron Q11 firmware described in
[README.md](./README.md). Nothing here is implemented yet.

- [Goals](#goals)
- [Decisions](#decisions)
- [Step 0: userspace layout and build CI](#step-0-userspace-layout-and-build-ci)
- [Step 1: make keymap.c the source of truth](#step-1-make-keymapc-the-source-of-truth)
- [Step 2: base color as a custom effect](#step-2-base-color-as-a-custom-effect)
- [Step 3: extract the color logic and test it](#step-3-extract-the-color-logic-and-test-it)
- [Step 4: Caps Lock overlay](#step-4-caps-lock-overlay)
- [Step 5: Fn overlay](#step-5-fn-overlay)
- [Step 6: update the README](#step-6-update-the-readme)
- [To verify against the QMK source](#to-verify-against-the-qmk-source)

## Goals

1. The per-layer color stops overriding the stock RGB effects. It becomes one
   effect among others, and Fn+Q/A can still switch to the stock ones.
2. Flashing no longer loses the layout. No more reloading `via.layout.json`
   after every flash.
3. With Caps Lock on, the keys it affects light up amber.
4. With Fn held, only the keys the Fn layer actually maps light up red. The
   others are dimmed instead of the whole board turning red.
5. The color logic is unit-tested on the computer, without a keyboard.

## Decisions

| Topic                          | Decision                                                                                   |
| :----------------------------- | :----------------------------------------------------------------------------------------- |
| Caps Lock color                | Amber, about `255, 140, 0`, scaled by brightness                                           |
| Keys affected by Caps Lock     | A–Z only. On macOS Swiss French, Caps Lock leaves è, é, à… unchanged                       |
| Fn layer, mapped keys          | Red (`139, 0, 0` as today), scaled by brightness                                           |
| Fn layer, fall-through keys    | The base color, dimmed. Start at 50% and adjust ("not too dimmed")                         |
| Fn layer, `KC_NO` keys         | Treated like fall-through keys (dimmed). They don't fall through, but they do nothing      |
| Caps Lock while Fn is held     | Fn overlay wins. The Caps Lock highlight comes back when Fn is released                    |
| Source of truth for the layout | `keymap.c`. VIA stays enabled for experimenting, but VIA edits are temporary until copied  |
| Repository layout              | QMK external userspace, with the firmware built by GitHub Actions                          |

## Step 0: userspace layout and build CI

Best done **first**. Every later step changes the C code, and a build on each
push catches compile errors before anything gets flashed. Moving the files now
also means the test setup in step 3 is written once, against the final paths.

QMK's "external userspace" (the
[`qmk/qmk_userspace`](https://github.com/qmk/qmk_userspace) template) is a
repository holding only your keymaps:

1. Move `keychron-q11-iso/keymap/` to
   `keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/`, and
   `via.layout.json` next to it. The folder name becomes the keymap name
   (`-km alphahydrae`); the path already says which keyboard it is for.
2. Add `qmk.json` listing the build target. `qmk userspace-add -kb
   keychron/q11/iso_encoder -km alphahydrae` writes it.
3. `qmk config user.overlay_dir="$(realpath .)"` points QMK at the repository.
   This replaces the symlink described in the README; remove the symlink.
4. Copy the template's GitHub Actions workflow
   (`.github/workflows/build_binaries.yaml`). It builds every target in
   `qmk.json` on each push and attaches the `.bin` to a GitHub release, so
   flashing doesn't need a local build.
5. Update the README: overlay setup instead of the symlink, new paths and
   keymap name, and where to download the CI-built firmware.

Step 3 adds the unit tests to this CI.

## Step 1: make keymap.c the source of truth

Today `keymaps[]` holds the stock layout and the real layout lives in VIA's
storage on the keyboard, which is reset by most flashes. VIA's storage starts
from `keymaps[]` when it's reset, so once `keymaps[]` holds the real layout, a
reset is harmless.

1. Generate the layout from the VIA backup with `qmk via2json`, which also
   reorders keys from matrix order to `LAYOUT_92_iso(...)` order. Paste the
   result into `keymaps[]` and `encoder_map[]`.
2. Replace the VIA macros:
   - `MACRO(2)` (Shift+Ctrl+Cmd+T) becomes the single keycode `LCSG(KC_T)`.
   - The empty `MACRO(3)`–`MACRO(5)` become `KC_NO`.
3. Decide on the layer 0 Home key (`KC_WWW_HOME` vs `KC_HOME`, see the
   README's Known quirks).
4. Use `get_highest_layer(layer_state | default_layer_state)` wherever the
   active layer is needed. This fixes the quirk where the Windows switch
   position shows layer 0's color.
5. Keep or delete `via.layout.json`. Once `keymaps[]` is the truth, it's only a
   reference.

This step also matters for steps 4 and 5. VIA only writes to the half plugged
into the computer, so the right half keeps using `keymaps[]`. If the two
differed, the overlays would look up different keys on each half.

## Step 2: base color as a custom effect

- `rules.mk`: `RGB_MATRIX_CUSTOM_USER = yes`
- New `rgb_matrix_user.inc` declaring `RGB_MATRIX_EFFECT(layer_color)`, which
  paints the per-layer base color (blue on layer 0, purple on layer 2).
- `config.h`: `#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_layer_color`
  so it's the starting effect.
- Fn+Q/A now cycles through it and the stock effects.

The overlays (steps 4 and 5) stay in `rgb_matrix_indicators_advanced_user()`.
That hook runs after **any** effect, so the Caps Lock and Fn highlights also
work on top of a stock effect.

While Fn is held, dimmed fall-through keys show the dimmed **base color**,
whatever effect is selected. The alternative (dimming whatever the effect drew)
needs reading back LED colors, which QMK doesn't really offer.

## Step 3: extract the color logic and test it

Split the code so that the decisions are plain C with no QMK dependencies,
and only a thin loop talks to QMK.

```text
keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/
  keymap.c            keymaps[], encoder_map[], the QMK hooks (thin)
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

Add a CI job that runs the tests on every push, next to the firmware build from
step 0.

Later, if tap-hold keys, combos or tap dance are added, QMK's own test
framework (`qmk_firmware/tests/`, GoogleTest) can simulate key sequences
through the real QMK pipeline.

## Step 4: Caps Lock overlay

- `host_keyboard_led_state().caps_lock` gives the Caps Lock state.
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

## Step 5: Fn overlay

While layer 1 or 3 is active:

- A key whose keycode **on the Fn layer** is mapped (`is_fn_mapped()`) is red.
- Other keys get the base color of the layer below, dimmed (50% to start).
- The lookup uses the Fn layer only, not `layer_switch_get_layer()`, because
  the question is "does Fn do something here", not "what does this key do
  now".

Possible later tweak: give the held Fn key its own color.

## Step 6: update the README

- Remove the VIA reload steps, and the Known quirks that are fixed (Windows
  switch color, hidden RGB effects).
- Describe the new lighting and the tests.

## To verify against the QMK source

Written from memory of QMK, not checked against the current source:

- The workflow file name and `qmk userspace-add` in the current `qmk_userspace`
  template and QMK CLI.
- `RGB_MATRIX_CUSTOM_USER` and the `RGB_MATRIX_CUSTOM_<name>` mode name.
- That Keychron's keyboard code for the Q11 doesn't paint its own indicators
  in a way that conflicts with `rgb_matrix_indicators_advanced_user()`.
- `qmk via2json` exists in the installed QMK version and handles this layout.
- `LCSG()` exists (otherwise `LCTL(LSFT(LGUI(KC_T)))`).
- `SPLIT_LED_STATE_ENABLE` is the right option for syncing Caps Lock.
- `quantum/keycodes.h` compiles on its own for the host tests.
- How the Keychron VIA definition lists custom effects (cosmetic only).
