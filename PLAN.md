# Plan: Caps Lock and Fn overlays

Planned changes to the Keychron Q11 firmware described in
[README.md](./README.md).

The Caps Lock overlay is done (see "Backlight color" and "Tests" in the
README). The Fn overlay remains.

- [Goals](#goals)
- [Decisions](#decisions)
- [Step 2: Fn overlay](#step-2-fn-overlay)
- [Step 3: update the README](#step-3-update-the-readme)
- [Migrating an install from the symlink setup](#migrating-an-install-from-the-symlink-setup)
- [To verify against the QMK source](#to-verify-against-the-qmk-source)

## Goals

1. With Caps Lock on, the keys it affects light up amber.
2. With Fn held, only the keys the Fn layer actually maps light up red. The
   others are dimmed instead of the whole board turning red.
3. The new color logic goes in `lighting.c` and is unit-tested in
   `test/test_lighting.c`, like the existing logic (see the README's Tests
   section).

## Decisions

| Topic                          | Decision                                                                                   |
| :----------------------------- | :----------------------------------------------------------------------------------------- |
| Caps Lock color                | Amber, about `255, 140, 0`, scaled by brightness                                           |
| Keys affected by Caps Lock     | A–Z only. On macOS Swiss French, Caps Lock leaves è, é, à… unchanged                       |
| Fn layer, mapped keys          | Red (`139, 0, 0` as today), scaled by brightness                                           |
| Fn layer, fall-through keys    | The base color, dimmed. Start at 50% and adjust ("not too dimmed")                         |
| Fn layer, `KC_NO` keys         | Treated like fall-through keys (dimmed). They don't fall through, but they do nothing      |
| Caps Lock while Fn is held     | Fn overlay wins. The Caps Lock highlight comes back when Fn is released                    |

## Step 2: Fn overlay

`key_color()` in `lighting.c` returns `false` for keys without an overlay, so
they keep the effect's color. For this step, it gets the Fn layer keycode as an
extra argument, and on the Fn layers it returns `true` for every key (red or
dimmed). The hook in `keymap.c` looks that keycode up with
`keymap_key_to_keycode(layer, key)` and passes it in.

While Fn is held, dimmed fall-through keys show the dimmed **base color**,
whatever effect is selected. The alternative (dimming whatever the effect drew)
needs reading back LED colors, which QMK doesn't really offer.

While layer 1 or 3 is active:

- A key whose keycode **on the Fn layer** is mapped (`is_fn_mapped()`) is red.
- Other keys get the base color of the layer below, dimmed (50% to start).
  `base_color()` already returns the color of the layer below for the Fn
  layers.
- The lookup uses the Fn layer only, not `layer_switch_get_layer()`, because
  the question is "does Fn do something here", not "what does this key do
  now".

In `lighting.c`: `is_fn_mapped(keycode)`, not `KC_TRNS` and not `KC_NO`, and
the Fn arguments of `key_color()`.

Test properties:

- Caps Lock gives amber only when Fn isn't held.
- With Fn held, mapped keys are red and the others are the base color, dimmed.
- Fall-through keys are dimmed but never black when brightness isn't 0
  (guards the "not too dimmed" intent against rounding).
- Without Fn or Caps Lock, every key keeps the effect's color (`key_color()`
  returns `false`).

Possible later tweak: give the held Fn key its own color.

Later, if tap-hold keys, combos or tap dance are added, QMK's own test
framework (`qmk_firmware/tests/`, GoogleTest) can simulate key sequences
through the real QMK pipeline.

## Step 3: update the README

- Remove the "Fn red covers the whole board" Known quirk.
- Describe the Fn overlay (the "While Fn is held, every key is dark red" bullet
  in "Backlight color"), and what the tests now cover.
- The table in "Backlight color" will be out of date.

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

- How the Keychron VIA definition lists custom effects (cosmetic only).
