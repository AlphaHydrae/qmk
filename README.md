# QMK: Keychron Q11 (ISO, with knobs)

Custom firmware and key layout for a [Keychron
Q11](https://www.keychron.com/products/keychron-q11-qmk-custom-mechanical-keyboard)
ISO split keyboard with two rotary encoders (knobs), built with
[QMK](https://qmk.fm).

[![build](https://github.com/AlphaHydrae/qmk/actions/workflows/build.yml/badge.svg)](https://github.com/AlphaHydrae/qmk/actions/workflows/build.yml)
[![MIT License](https://img.shields.io/static/v1?label=license&message=MIT&color=informational)](./LICENSE.txt)

Everything lives in the keymap folder,
[`keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/`](./keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/):

| File                  | What it holds                                                         |
| :-------------------- | :-------------------------------------------------------------------- |
| `keymap.c`            | The key layout, the knobs, and the Fn and Caps Lock overlays          |
| `rgb_matrix_user.inc` | The `layer_color` RGB effect: the per-layer backlight color           |
| `lighting.h` / `.c`   | The color logic itself, plain C (tested)                              |
| `fingerprint.h` / `.c`| The keymap fingerprint that reloads VIA's copy on change (tested)     |
| `layers.h`            | The layer names, shared by the files above                            |
| `rules.mk`            | Which QMK features are built in                                       |
| `config.h`            | Split keyboard settings, and the RGB effect to start with             |

It is compiled with QMK and flashed over USB, on each half. `keymap.c` is the
source of truth for the layout. VIA is still enabled for experimenting, but VIA
edits are temporary (see [How firmware and VIA
interact](#how-firmware-and-via-interact)).

- [What the firmware does](#what-the-firmware-does)
- [The layout](#the-layout)
- [How firmware and VIA interact](#how-firmware-and-via-interact)
- [Repository layout and CI](#repository-layout-and-ci)
- [Installing](#installing)
- [Making further changes](#making-further-changes)
- [Known quirks](#known-quirks)
- [Troubleshooting](#troubleshooting)

## What the firmware does

The keymap folder started as a copy of Keychron's stock `default` keymap for
`keychron/q11/iso_encoder` (from the QMK repository).

### Backlight color

The backlight color depends on the layer, in two parts.

**The `layer_color` effect**
([`rgb_matrix_user.inc`](./keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/rgb_matrix_user.inc))
is a custom RGB effect, like QMK's stock ones (breathing, rainbow…). It paints
every key blue, or purple when the "brackets" layer is on, either toggled with
`TG(BRACKETS)` or as the default layer on the Windows switch position. It's the
effect the firmware starts with, and Fn+Q/A cycles through it and the stock
effects (it comes after the last stock one). The bottom left key (`LYR_RGB`)
switches straight back to it, and turns the backlight on if it was off. That's
a custom keycode, handled by `process_record_user()` in `keymap.c`.

**The Fn and Caps Lock overlays** come from
`rgb_matrix_indicators_advanced_user()` in `keymap.c`. QMK defines a do-nothing
("weak") version of this function and calls it on every frame, after the current
effect has been drawn. Defining your own version replaces it, a bit like
overriding a hook. QMK calls it with a range of LEDs (`led_min` to `led_max`)
because it spreads the drawing over several calls. Here it draws on top of
whatever effect is selected:

- While Fn is held, the keys the Fn layer maps are dark red, and so are both Fn
  keys. Those are `_______` on the Fn layer, so they're recognized by their
  `MO()` keycode on the layer below. The others (the Fn layer's other `_______`
  and `XXXXXXX` keys) step back so the red keys stand out:
  - With the `layer_color` effect, they show the base color of the layer below
    (blue or purple), dimmed to about half. The dimming rounds up, so these keys
    never go black unless the backlight is at brightness 0. Its strength is
    `FN_DIM` in `lighting.c`.
  - With any other effect, they're black. Dimming what the effect drew would
    need reading back the LED colors, which QMK doesn't offer, and the dimmed
    blue or purple would look out of place over a rainbow.
- Otherwise, while Caps Lock is on, the letter keys (`KC_A` to `KC_Z`) and the
  Caps Lock key itself are amber (`255, 140, 0`). With the Mac's input source
  set to Swiss French, Caps Lock doesn't change the accented keys (è, é, à…), so
  they aren't highlighted. The amber comes back when Fn is released.
- The other keys keep the color the effect drew.

The function decides per key, from two keycodes. For Caps Lock, its keycode on
the layer actually in effect for it (`layer_switch_get_layer()`, then
`keymap_key_to_keycode()`), so a transparent key counts as the key of the layer
below. For Fn, its keycode on the Fn layer itself, without falling through,
because the question there is whether Fn does something for that key. The
first keycode also tells which keys are the Fn keys (`MO()`). Both
halves do this lookup with their own copy of the keymap, so they must use the
same keymap (see [How firmware and VIA
interact](#how-firmware-and-via-interact): VIA edits only reach the left half).

| Active layer               | Color set (R, G, B)       | Looks like               | Drawn by      |
| :------------------------- | :------------------------ | :----------------------- | :------------ |
| 0 (`BASE`)                 | `0, 64, 128`              | blue                     | `layer_color` |
| 1 (`FN`, Fn held)          | `139, 0, 0` / `0, 33, 65` | dark red / dimmed blue   | `keymap.c`    |
| 2 (`BRACKETS`)             | `64, 0, 64`               | purple                   | `layer_color` |
| 3 (`BRACKETS_FN`, Fn held) | `139, 0, 0` / `33, 0, 33` | dark red / dimmed purple | `keymap.c`    |

On the Fn layers, the red is for the keys the Fn layer maps and the Fn keys,
the dimmed color for the others (black with an effect other than
`layer_color`).

All the colors are scaled by the backlight brightness (`rgb_matrix_get_val()`,
0–255: `value * brightness / 255`), so the brightness keys and knob still work.
The layers include the default layer set by the Mac/Windows switch
(`layer_state | default_layer_state`).

The colors themselves are decided in
[`lighting.c`](./keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/lighting.c),
whose only QMK header is `quantum/keycodes.h` (the keycode values, a
standalone header). `rgb_matrix_user.inc` and `keymap.c` only read the
keyboard's state (active layer, keycodes, Caps Lock, brightness), ask
`lighting.c` for the color, and set the LEDs. This keeps the logic testable on
the computer (see [Tests](#tests)).

The function returns `false`, which stops the keyboard's own code (`q11.c` in
QMK) from drawing its indicator on top (it would turn the Caps Lock key white
while Caps Lock is on).

### `rules.mk`: build features

```make
ENCODER_MAP_ENABLE = yes  # knobs are configured per layer (encoder_map[]); same as stock
RGB_MATRIX_ENABLE = yes   # per-key RGB (already enabled by the keyboard, harmless)
VIA_ENABLE = yes          # lets the VIA web app read/write the keymap (for experimenting)
RGB_MATRIX_CUSTOM_USER = yes  # adds the effects from rgb_matrix_user.inc

SRC += lighting.c  # compiles lighting.c into the firmware
```

Keymaps in upstream QMK don't enable VIA, so without `VIA_ENABLE` the web app
can't talk to this firmware.

### `config.h`: split keyboard sync and starting effect

The Q11 is a split keyboard. Each half has its own microcontroller and drives
its own LEDs, and only the half plugged into the computer (the "master", the
left one) knows which layer is active and whether Caps Lock is on. Because the
color code runs on **both** halves, that state has to be sent to the other
half:

```c
#define SPLIT_LAYER_STATE_ENABLE  // sync active layer to the other half (needed for the colors)
#define SPLIT_LED_STATE_ENABLE    // sync Caps Lock to the other half (needed for the Caps Lock overlay)
#define SPLIT_TRANSPORT_MIRROR    // mirror key presses to the other half (only matters for reactive RGB effects)
#define SPLIT_RGB_MATRIX_ENABLE   // not a recognized QMK option (defined twice); has no effect
```

Without `SPLIT_LAYER_STATE_ENABLE`, the right half would stay the layer-0 color.
Without `SPLIT_LED_STATE_ENABLE`, the right half's letters (Y, U, H, J, N, M…)
wouldn't turn amber.
The RGB settings (effect, brightness…) are synced without any option, because
the keyboard declares how many LEDs each half has.

```c
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_layer_color
```

This makes `layer_color` the starting effect. It only applies when the RGB
settings stored in the EEPROM are reset (see [How firmware and VIA
interact](#how-firmware-and-via-interact)).

## The layout

Compared to the stock keymap:

### Layer 0: base (`BASE`, Mac switch position)

- **Top row is plain F1–F12** (stock has the media/brightness keys here; they
  moved to the Fn layer).
- **ISO macOS key swap**: the key left of `1` sends `KC_NUBS` and the key
  right of left Shift sends `KC_GRV`. This swaps them back to the positions
  printed on the keycaps, fixing a well-known macOS quirk with ISO keyboards.
- **Left column** (the 5 extra keys on the far left, top to bottom):
  1. `TG(BRACKETS)`: toggle layer 2 on/off (backlight turns purple/blue).
  2. to 4. `XXXXXXX` (`KC_NO`): unused, they do nothing.
  5. `LYR_RGB`: switch the backlight back to the `layer_color` effect (see
     [Backlight color](#backlight-color)).
- Both Fn keys are `MO(FN)`, which activates layer 1 while held.
- **Knobs**: left = backlight brightness, right = volume. Pressing either knob
  = mute.

### Layer 1: Fn (`FN`, while Fn is held on layer 0)

| Key                 | Action                                         |
| :------------------ | :--------------------------------------------- |
| F1 / F2             | Screen brightness down / up                    |
| F3 / F4             | Mission Control / Launchpad                    |
| F5 / F6             | Backlight dimmer / brighter                    |
| F7 / F8 / F9        | Previous / play-pause / next track             |
| F10 / F11 / F12     | Mute / volume down / volume up                 |
| Del                 | Ctrl+Power (macOS: shut down / restart dialog) |
| Backspace           | Forward delete                                 |
| Tab                 | Backlight on/off                               |
| Q / A               | Next / previous RGB effect                     |
| W / S               | Backlight brighter / dimmer                    |
| E / D               | Hue up / down                                  |
| R / F               | Saturation up / down                           |
| T / G               | Effect speed up / down                         |
| N                   | Toggle NKRO (n-key rollover)                   |
| Knob press (either) | Backlight on/off                               |
| Knobs               | Backlight brightness (both)                    |

### Layer 2: "brackets" layer (`BRACKETS`, toggled with the first left key)

Same as layer 0, except:

- The keys that hold brackets on a US layout send the **macOS Option
  combinations that type brackets on a Swiss keyboard layout**. This assumes the
  Mac's input source is set to Swiss.

  | Physical key (ISO)          | Sends        | Types (Swiss Mac layout) |
  | :-------------------------- | :----------- | :----------------------- |
  | 1st key right of `P`        | `A(KC_5)`    | `[`                      |
  | 2nd key right of `P`        | `A(KC_6)`    | `]`                      |
  | 1st key right of `L`        | `LSA(KC_7)`  | `\`                      |
  | 2nd key right of `L`        | `A(KC_8)`    | `{`                      |
  | Key left of Enter (`#`/`$`) | `A(KC_9)`    | `}`                      |

- Fn keys are `MO(BRACKETS_FN)` instead of `MO(FN)`.
- **Knobs**: both control volume.

Layer 2 is also the default layer when the Mac/Windows switch is on
**Windows** (see `q11.c` in QMK), so that position gives the brackets layer
permanently.

### Layer 3: Fn while on layer 2 (`BRACKETS_FN`)

Identical to layer 1.

## How firmware and VIA interact

When VIA is enabled, the keyboard doesn't use `keymaps[]` from `keymap.c`
directly. It keeps a copy of the keymap, knobs and macros in its persistent
memory (EEPROM), and that copy is what VIA edits and what the keyboard uses.

`keymap.c` only provides the **initial values** of that copy, and the copy is
reset to them:

- when you flash a firmware whose **`keymaps[]` or `encoder_map[]` changed**. At
  startup, `keyboard_post_init_user()` in `keymap.c` computes a fingerprint of
  both
  ([`fingerprint.c`](./keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/fingerprint.c))
  and compares it with the one saved in the EEPROM's user slot
  (`eeconfig_read_user()`). If they differ, it reloads the copy
  (`dynamic_keymap_reset()`) and saves the new fingerprint. Flashing a firmware
  with an unchanged layout keeps the copy, VIA edits included.
- when you flash a firmware **built on a different day** than the one currently
  on the keyboard. That's QMK's own check: it derives a "validity" stamp from
  the build date only, so on its own it misses layout changes flashed on the
  same day, which is why the fingerprint check exists.
- when you enter the bootloader by holding Esc/Del while plugging in, if that
  works for you (see [Flash](#flash-both-halves)). This also clears the EEPROM.
  The reset button doesn't clear it by itself.

The RGB settings (effect, brightness, hue…) are stored in the EEPROM too, but
the first two resets don't touch them: they survive a flash. Only the Esc/Del
reset clears them, back to the defaults in `config.h`. VIA macros are only
cleared by the build-date and Esc/Del resets.

Since `keymap.c` holds the real layout, a reset is harmless. The flip side is
that **VIA edits are temporary**: they last until the next reset, and they only
apply to the half plugged into the computer (VIA writes to that half's EEPROM
only, the other half keeps using `keymap.c`). To keep a change, copy it into
`keymaps[]` (see [Making further changes](#making-further-changes)).

## Repository layout and CI

This repository is a QMK "external userspace", based on the
[`qmk/qmk_userspace`](https://github.com/qmk/qmk_userspace) template. It holds
only the keymap, and QMK's own tree (`~/qmk_firmware`) stays untouched:

- The keymap lives where it would in QMK's tree,
  `keyboards/<keyboard>/keymaps/<name>/`. The folder name is the keymap name
  (`-km alphahydrae`).
- [`qmk.json`](./qmk.json) lists the build targets (keyboard + keymap).
  `qmk userspace-add` / `qmk userspace-remove` edit it.
- [`.github/workflows/build_binaries.yaml`](./.github/workflows/build_binaries.yaml)
  builds every target on each push, against the latest upstream `qmk_firmware`.
  Each run's firmware is downloadable from its page in the GitHub Actions tab.
  Successful builds of `main` are also published as the [`latest`
  release](https://github.com/AlphaHydrae/qmk/releases/tag/latest), replacing
  the previous one. The template publishes after every push, even failed ones
  and other branches. This repository doesn't. The same workflow also runs the
  [tests](#tests), and doesn't publish if they fail.
- [`test/`](./test/) holds the unit tests of the color logic and the keymap
  fingerprint.
- The [`Makefile`](./Makefile) has shortcuts for the common tasks:

  | Command        | Does                                                      |
  | :------------- | :-------------------------------------------------------- |
  | `make`         | `make test`, then `make compile`                          |
  | `make test`    | Builds and runs the unit tests (no `qmk` tool needed)     |
  | `make compile` | `qmk compile` for this keyboard and keymap                |
  | `make flash`   | `qmk flash` for this keyboard and keymap (one half)       |
  | `make clean`   | Deletes the test build and the firmware files             |

### Tests

[`test/test_lighting.c`](./test/test_lighting.c) tests `lighting.c` on the
computer, without a keyboard or the `qmk` tool, using the computer's own C
compiler. They only need QMK's keycode values, from `quantum/keycodes.h` in a
`qmk_firmware` checkout (`~/qmk_firmware` by default; CI checks out just that
file):

```bash
make test
make test QMK_HOME=/path/to/qmk_firmware
```

The input space is small (4 layers, 65,536 keycodes, Caps Lock on or off, 256
brightness levels), so the tests check every combination rather than random
samples, about 3.5 billion checks in about three seconds. `key_color()` takes two
keycodes, but only one matters on a given layer (the Fn layer keycode on the Fn
layers, the other one elsewhere), so every value of that one is checked against
a few samples of the other, with the `layer_color` effect on and off:

- The documented colors at full brightness.
- Every channel is the full color scaled by the brightness and never exceeds
  it, and brightness 0 gives black.
- Exactly the 26 letter keycodes are affected by Caps Lock, and those plus
  `KC_CAPS` are highlighted.
- Exactly two keycodes count as unmapped on the Fn layer: `KC_TRNS` and
  `KC_NO`. Exactly the 32 `MO()` keycodes count as Fn keys.
- The dimmed base color is about half the base color, never black unless the
  brightness is 0, and visibly dimmer than the base color.
- On the Fn layers, every key gets the overlay, whatever the Caps Lock state:
  dark red if the Fn layer maps it or it's an Fn key, otherwise the dimmed base
  color with `layer_color` and black with any other effect. Outside the Fn
  layers, the Fn keys keep the effect's color.
- Otherwise, a key is amber exactly when Caps Lock is on and it's a letter or
  the Caps Lock key, and every other key keeps the effect's color.

[`test/test_fingerprint.c`](./test/test_fingerprint.c) tests `fingerprint.c`.
The property that matters is that changing any single keycode changes the
fingerprint, so a layout change always reloads VIA's copy:

- One step of the fingerprint maps all 65,536 keycodes to different values, for
  several starting values. This is why a single change can't go unnoticed.
- In a keymap-sized array, each position changed to 512 other keycodes changes
  the fingerprint.
- Chaining calls (`keymaps[]`, then `encoder_map[]`) gives the same result as
  one call over both.

Both programs share the small `CHECK()` harness in
[`test/check.h`](./test/check.h).

The firmware code that reads the keyboard's state, sets the LEDs and compares
the fingerprint with the EEPROM isn't covered, so these changes still need a
check on the keyboard.

## Installing

### One-time setup (macOS)

```bash
# QMK command line tool. Homebrew asks to trust third-party taps first: qmk/qmk
# is QMK's official tap, osx-cross/arm and osx-cross/avr provide the compilers.
brew trust qmk/qmk
brew install qmk/qmk/qmk

# Clones QMK into ~/qmk_firmware and installs the compiler toolchain and the
# flashing tools (dfu-util, etc.). Answer yes to the prompts.
qmk setup

# Homebrew doesn't put the compilers and the ARM binutils on the PATH
# ("keg-only"), and dos2unix isn't installed by the above. Without this,
# `qmk doctor` reports "Can't find arm-none-eabi-gcc in your path" (or crashes
# on a missing `arm-none-eabi-size`) and the build fails. The Q11 only needs
# the ARM tools; avr-gcc is for older AVR keyboards.
brew link --force arm-none-eabi-gcc@8 arm-none-eabi-binutils avr-gcc@8
brew install dos2unix
qmk doctor

# Optional: a GUI flashing tool, as an alternative to `qmk flash`
brew install qmk-toolbox
```

Check that the stock firmware compiles:

```bash
qmk compile -kb keychron/q11/iso_encoder -km default
```

### Point QMK at this repository

QMK normally looks for keymaps inside its own tree. Tell it to also look in
this repository (run from the repository's root):

```bash
qmk config user.overlay_dir="$(realpath .)"
```

Optionally, make it the default so you can omit `-kb` and `-km`:

```bash
qmk config user.keyboard=keychron/q11/iso_encoder user.keymap=alphahydrae
```

### Compile

```bash
qmk compile -kb keychron/q11/iso_encoder -km alphahydrae
```

This produces `keychron_q11_iso_encoder_alphahydrae.bin` in this repository's
root (ignored by Git). Compiler errors show up here, so it's worth compiling
before trying to flash. `qmk userspace-compile` builds every target in
`qmk.json` at once.

Instead of compiling locally, you can also download the `.bin` built by CI
(see [Repository layout and CI](#repository-layout-and-ci)) and flash it with
QMK Toolbox.

### Flash (both halves)

Both halves run the same firmware and must be flashed **one at a time**, each
with the USB cable plugged directly into it from the computer. A half only
accepts new firmware while in bootloader (DFU) mode.

To put a half in bootloader mode, use the **reset button**. It's a small
button on the circuit board, under the **space bar keycap** of each half. Pull
the space bar keycap off (a keycap puller works), then press the button with
something pointy (the end of the keycap puller, a SIM-eject pin, a paperclip)
**while plugging in** the USB cable. Release once it's plugged in.

1. **Left half**: disconnect the two halves from each other, and unplug the
   keyboard.
2. Run:

   ```bash
   qmk flash -kb keychron/q11/iso_encoder -km alphahydrae
   ```

   It compiles if needed, then waits, printing that it can't find a bootloader.
3. Hold the **reset button** of the **left half** and plug the USB cable into
   it. The flash starts. The backlight stays off while in bootloader mode. Wait
   for it to finish.
4. **Right half**: unplug, run the same `qmk flash` command again, then hold
   the right half's reset button and plug the USB cable into the **right
   half**.
5. Reconnect the halves, put the space bar keycaps back, and plug the left half
   into the computer as usual.

The keyboard's readme in QMK also describes a shortcut: hold **Esc** (left
half) or **Del** (right half) while plugging in, instead of using the reset
button. That relies on the firmware already on the keyboard supporting it, so
the reset button is the reliable method.

(With QMK Toolbox instead: open the `.bin` file, tick "Auto-Flash", then do the
reset button + plug-in steps for each half.)

### Check the result

Check that the first left key toggles the backlight between blue and purple.
If the keyboard still shows the effect it had before flashing (RGB settings
survive a flash), press the bottom left key to switch to `layer_color`. The
choice is saved.

## Making further changes

- Edit the files in [the keymap
  folder](./keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/), then
  compile and flash both halves. This covers remapping keys and knobs, as well
  as anything VIA can't do (custom behavior, lighting logic, tap/hold keys,
  combos, etc.).
  - Keys are listed in `LAYOUT_92_iso(...)` order, which follows the physical
    rows. `_______` (`KC_TRNS`) falls through to the layer below, `XXXXXXX`
    (`KC_NO`) does nothing.
  - New features are usually turned on in `rules.mk` (`SOMETHING_ENABLE = yes`),
    tuned in `config.h` (`#define ...`), and used in `keymap.c`.
  - Code that depends on the layer or on key presses must work on the right half
    too. See the [split keyboard
    docs](https://docs.qmk.fm/features/split_keyboard) for what has to be
    synced.
  - The tests in `test/` only cover the plain C logic. If tap-hold keys, combos
    or tap dance are added, QMK's own test framework (`qmk_firmware/tests/`,
    GoogleTest) can simulate key sequences through the real QMK pipeline.
  - Useful docs: [RGB matrix](https://docs.qmk.fm/features/rgb_matrix),
    [keycodes](https://docs.qmk.fm/keycodes),
    [layers](https://docs.qmk.fm/feature_layers).
- **Trying a change in VIA first** is fine (no recompiling), but copy the result
  into `keymaps[]` afterwards, or it's lost on the next reset. VIA runs at
  [usevia.app](https://usevia.app) in Chrome (it needs WebHID). If it doesn't
  recognize the keyboard, Keychron publishes a VIA definition JSON for the Q11
  ISO knob version on its website, which can be loaded in VIA's **Settings →
  Show Design tab → Design → Load**. To turn a VIA
  backup ("Save + Load → Save Current Layout") into C, `qmk via2json -kb
  keychron/q11/iso_encoder -l LAYOUT_92_iso <backup.json>` converts it to a QMK
  keymap JSON in layout order. It doesn't convert the knobs, and some keycode
  names are older than the ones `keymap.c` uses (`RGB_*` instead of `RM_*`,
  `KC_POWER` instead of `KC_KB_POWER`). The VIA backup used to build the
  current `keymap.c` is in the Git history
  (`keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae/via.layout.json`).

## Known quirks

- **Layers 1 and 3 share the same red**, so the color doesn't tell you which Fn
  layer is active.
- **On the Windows switch position**, layer 2 is already the default, so the
  `TG(BRACKETS)` key has no visible effect.
- **The `layer_color` effect ignores the hue, saturation and speed keys**
  (Fn+E/D, R/F, T/G). They still change the stock effects.
- `SPLIT_RGB_MATRIX_ENABLE` is defined twice in `config.h` and isn't a current
  QMK option. It can be removed.

## Troubleshooting

- **`qmk: bad interpreter: .../python: no such file or directory`**, or `brew
  install` ending with **`qmk/qmk/qmk: No such file or directory -
  /opt/homebrew/bin/qmk`**: the `qmk` tool runs in its own Python environment,
  built against a specific Python version (see
  `/opt/homebrew/Cellar/qmk/*/libexec/pyvenv.cfg`). That version isn't
  installed, either because Homebrew removed it when upgrading, or because the
  prebuilt package targets an older version than the current Homebrew Python. A
  plain `brew reinstall` doesn't help. Either install the missing version and
  reinstall (`brew install python@3.13 && brew reinstall qmk/qmk/qmk`), or
  rebuild `qmk` against the current Python with `brew reinstall
  --build-from-source qmk/qmk/qmk`.
- **`qmk flash` keeps waiting for the bootloader**: make sure the reset button
  is held *before* the cable is plugged in, that the cable goes straight to the
  half being flashed, and that the cable carries data.
- **Keys don't match `keymap.c` after flashing**: the EEPROM still holds an
  older VIA copy of the layout, probably because the previous firmware was
  built on the same day. Clear it by holding Esc (left half) or Del (right
  half) while plugging in (see [How firmware and VIA
  interact](#how-firmware-and-via-interact)).
- **Going back to stock**: flash with `-km default` (note that the upstream
  default keymap has no VIA support), or download Keychron's official firmware
  for the Q11 from their website and flash it the same way.
