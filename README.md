# QMK: Keychron Q11 (ISO, with knobs)

Custom firmware and key layout for a [Keychron
Q11](https://www.keychron.com/products/keychron-q11-qmk-custom-mechanical-keyboard)
ISO split keyboard with two rotary encoders (knobs). It is built with
[QMK](https://qmk.fm).

The customization lives in two separate places, and they are installed in two
different ways:

| What                        | Where                                                                    | How it gets on the keyboard                  |
| :-------------------------- | :----------------------------------------------------------------------- | :------------------------------------------- |
| **Firmware** (C code)       | [`keychron-q11-iso/keymap/`](./keychron-q11-iso/keymap/)                 | Compile with QMK, flash over USB (each half) |
| **Key layout** (VIA backup) | [`keychron-q11-iso/via.layout.json`](./keychron-q11-iso/via.layout.json) | Load in the VIA web app after flashing       |

The firmware adds things the web app can't do: the **backlight color changes
with the active layer**. The VIA layout holds the key remapping, macros and knob
assignments, which the web app *can* edit.

- [What the firmware does](#what-the-firmware-does)
- [What the VIA layout does](#what-the-via-layout-does)
- [How firmware and VIA interact](#how-firmware-and-via-interact)
- [Installing](#installing)
- [Making further changes](#making-further-changes)
- [Known quirks](#known-quirks)
- [Troubleshooting](#troubleshooting)

## What the firmware does

The keymap folder is a copy of Keychron's stock `default` keymap for
`keychron/q11/iso_encoder` (from the QMK repository) with these changes.

### `keymap.c`: per-layer backlight color

The key definitions (`keymaps[]`, `encoder_map[]`) are **unchanged** from the
stock keymap. The only addition is this function:

```c
bool rgb_matrix_indicators_user(void) { ... }
```

QMK defines a do-nothing ("weak") version of this function and calls it on every
frame after the current RGB effect has been drawn. Defining your own version
replaces it, a bit like overriding a hook. Here it:

1. Reads the current backlight brightness (`rgb_matrix_get_val()`, 0–255), so
   the brightness keys and knob still dim/brighten the color.
2. Finds the highest active layer (`get_highest_layer(layer_state)`).
3. Paints **every key** a solid color for that layer, scaled by the brightness
   (that's what the `SCALE()` macro does: `value * brightness / 255`).

| Active layer             | Color set (R, G, B) | Looks like |
| :----------------------- | :------------------ | :--------- |
| 0 (`MAC_BASE`)           | `0, 64, 128`        | blue       |
| 1 (`MAC_FN`, Fn held)    | `139, 0, 0`         | dark red   |
| 2 (`WIN_BASE`)           | `64, 0, 64`         | purple     |
| 3 (`WIN_FN`, Fn held)    | `139, 0, 0`         | dark red   |

Combined with the VIA layout below, the backlight tells you at a glance whether
the toggled "brackets" layer (layer 2) is on (purple) or off (blue), and turns
red while Fn is held.

The function returns `false`, which tells the keyboard-level code not to draw
its own indicators on top.

### `rules.mk`: build features

```make
ENCODER_MAP_ENABLE = yes  # knobs are configured per layer (encoder_map[]); same as stock
RGB_MATRIX_ENABLE = yes   # per-key RGB (already enabled by the keyboard, harmless)
VIA_ENABLE = yes          # lets the VIA web app read/write the keymap
```

`VIA_ENABLE` is the important one. Keymaps in upstream QMK don't enable VIA, so
without it the web app can't talk to your custom firmware.

### `config.h`: split keyboard sync

The Q11 is a split keyboard. Each half has its own microcontroller and drives
its own LEDs, and only the half plugged into the computer (the "master", the
left one) knows which layer is active. Because the color code runs on **both**
halves, the layer state has to be sent to the other half:

```c
#define SPLIT_LAYER_STATE_ENABLE  // sync active layer to the other half (needed for the colors)
#define SPLIT_TRANSPORT_MIRROR    // mirror key presses to the other half (only matters for reactive RGB effects)
#define SPLIT_RGB_MATRIX_ENABLE   // not a recognized QMK option (defined twice); has no effect
```

Without `SPLIT_LAYER_STATE_ENABLE`, the right half would stay the layer-0 color.

## What the VIA layout does

[`via.layout.json`](./keychron-q11-iso/via.layout.json) is a backup exported
from the VIA web app ("Save + Load" → "Save Current Layout"). It stores 4
layers, the knob assignments and the macros. Compared to the stock keymap:

### Layer 0: base (Mac switch position)

- **Top row is plain F1–F12** (stock has the media/brightness keys here; they
  moved to the Fn layer).
- **ISO macOS key swap**: the key left of `1` sends `KC_NUBS` and the key
  right of left Shift sends `KC_GRV`. This swaps them back to the positions
  printed on the keycaps, fixing a well-known macOS quirk with ISO keyboards.
- **Left macro column** (the 5 extra keys on the far left, top to bottom):
  1. `TG(2)`: toggle layer 2 on/off (backlight turns purple/blue).
  2. `MACRO(2)`: Shift+Ctrl+Cmd+T.
  3. `MACRO(3)`, 4. `MACRO(4)`, 5. `MACRO(5)`: assigned, but those macros are
     empty, so they do nothing yet.
- **Home key** sends `KC_WWW_HOME` (browser home) instead of Home. See
  [Known quirks](#known-quirks).
- Both Fn keys are `MO(1)`, which activates layer 1 while held.
- **Knobs**: left = backlight brightness, right = volume. Pressing either knob
  = mute.

### Layer 1: Fn (while Fn is held on layer 0)

| Key                                  | Action                                         |
| :----------------------------------- | :--------------------------------------------- |
| F1 / F2                              | Screen brightness down / up                    |
| F3 / F4                              | `CUSTOM(0)` / `CUSTOM(1)`: Keychron's custom keycodes (Mission Control / Launchpad in the stock keymap) |
| F5 / F6                              | Backlight dimmer / brighter                    |
| F7 / F8 / F9                         | Previous / play-pause / next track             |
| F10 / F11 / F12                      | Mute / volume down / volume up                 |
| Del                                  | Ctrl+Power (macOS: shut down / restart dialog) |
| Backspace                            | Forward delete                                 |
| Tab                                  | Backlight on/off                               |
| Q / A                                | Next / previous RGB effect                     |
| W / S                                | Backlight brighter / dimmer                    |
| E / D                                | Hue up / down                                  |
| R / F                                | Saturation up / down                           |
| T / G                                | Effect speed up / down                         |
| N                                    | Toggle NKRO (n-key rollover)                   |
| Knob press (either)                  | Backlight on/off                               |
| Knobs                                | Backlight brightness (both)                    |

### Layer 2: "brackets" layer (toggled with the first macro key)

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

- The Home key is a real Home.
- Fn keys are `MO(3)` instead of `MO(1)`.
- **Knobs**: both control volume.

### Layer 3: Fn while on layer 2

Identical to layer 1.

## How firmware and VIA interact

When VIA is enabled, the keyboard doesn't use `keymaps[]` from `keymap.c`
directly. It keeps a copy of the keymap, knobs and macros in its persistent
memory (EEPROM), and that copy is what VIA edits and what the keyboard uses.

`keymap.c` only provides the **initial values** of that copy, and the copy is
reset to them:

- when you flash a firmware **built on a different day** than the one
  currently on the keyboard (QMK derives a "validity" stamp from the build
  date), and
- when you enter the bootloader by holding Esc/Del while plugging in, if that
  works for you (see [Flash](#flash-both-halves)). This also clears the EEPROM.
  The reset button doesn't clear it by itself.

So **after every flash, the VIA layout is lost** and has to be reloaded from
[`via.layout.json`](./keychron-q11-iso/via.layout.json). The RGB color code is
not affected by this, since it's compiled into the firmware.

## Installing

### One-time setup (macOS)

```bash
# QMK command line tool
brew install qmk/qmk/qmk

# Clones QMK into ~/qmk_firmware and installs the compiler toolchain and the
# flashing tools (dfu-util, etc.). Answer yes to the prompts.
qmk setup

# Optional: a GUI flashing tool, as an alternative to `qmk flash`
brew install qmk-toolbox
```

Check that the stock firmware compiles:

```bash
qmk compile -kb keychron/q11/iso_encoder -km default
```

### Link the keymap into QMK

QMK looks for keymaps inside its own tree, under
`keyboards/<keyboard>/keymaps/<name>/`. Instead of copying the files there,
symlink this repository's keymap folder so edits here are picked up directly:

```bash
ln -s ~/path/to/qmk/keychron-q11-iso/keymap \
  ~/qmk_firmware/keyboards/keychron/q11/iso_encoder/keymaps/alphahydrae-keychron-q11-iso
```

The link name (`alphahydrae-keychron-q11-iso`) becomes the keymap name used in
the commands below.

Optionally, make it the default so you can omit `-kb` and `-km`:

```bash
qmk config user.keyboard=keychron/q11/iso_encoder user.keymap=alphahydrae-keychron-q11-iso
```

### Compile

```bash
qmk compile -kb keychron/q11/iso_encoder -km alphahydrae-keychron-q11-iso
```

This produces
`~/qmk_firmware/keychron_q11_iso_encoder_alphahydrae-keychron-q11-iso.bin`.
Compiler errors show up here, so it's worth compiling before trying to flash.

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
   qmk flash -kb keychron/q11/iso_encoder -km alphahydrae-keychron-q11-iso
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

### Restore the VIA layout

1. Open [usevia.app](https://usevia.app) in Chrome (it needs WebHID) and
   authorize the keyboard.
2. In **Configure**, go to the **Save + Load** tab, click **Load Saved
   Layout**, and pick
   [`keychron-q11-iso/via.layout.json`](./keychron-q11-iso/via.layout.json).
3. Check that the first macro key toggles the backlight between blue and
   purple.

If VIA doesn't recognize the keyboard, Keychron publishes a VIA definition
JSON for the Q11 ISO knob version on its website, which can be loaded in VIA's
**Settings → Show Design tab → Design → Load**.

## Making further changes

- **Remapping keys, macros or knobs**: use VIA, no need to recompile. Afterwards
  use **Save + Load → Save Current Layout** and overwrite
  [`via.layout.json`](./keychron-q11-iso/via.layout.json) so the backup stays
  current.
- **Anything VIA can't do** (custom behavior, lighting logic, tap/hold keys,
  combos, etc.): edit the files in
  [`keychron-q11-iso/keymap/`](./keychron-q11-iso/keymap/), then compile, flash
  both halves, and reload the VIA layout.
  - New features are usually turned on in `rules.mk` (`SOMETHING_ENABLE = yes`),
    tuned in `config.h` (`#define ...`), and used in `keymap.c`.
  - Code that depends on the layer or on key presses must work on the right half
    too. See the [split keyboard
    docs](https://docs.qmk.fm/features/split_keyboard) for what has to be
    synced.
  - Useful docs: [RGB matrix](https://docs.qmk.fm/features/rgb_matrix),
    [keycodes](https://docs.qmk.fm/keycodes),
    [layers](https://docs.qmk.fm/feature_layers).
- If you want `keymap.c` to be the source of truth instead of VIA, copy the VIA
  layers into `keymaps[]`. The VIA file lists keys in matrix order (12 rows × 9
  columns: rows 0–5 are the left half, 6–11 the right half), not in the visual
  `LAYOUT_92_iso(...)` order.

## Known quirks

- **Layers 1 and 3 share the same red**, so the color doesn't tell you which Fn
  layer is active.
- **The Mac/Windows switch** on the keyboard selects the *default* layer (0 on
  Mac, 2 on Windows). The color code looks at `layer_state`, which doesn't
  include the default layer. So on the Windows position the board shows layer
  0's blue even though layer 2's keys are active, and the `TG(2)` key has no
  visible effect. This setup assumes the switch stays on **Mac**.
- **RGB effects are hidden**: since every key is painted a solid color on each
  frame, the effect chosen with Fn+Q/A isn't visible. Brightness, and turning
  the backlight on/off, still work.
- **Home key on layer 0** sends browser-home (`KC_WWW_HOME`) rather than Home,
  unlike layer 2. This might be unintentional.
- `SPLIT_RGB_MATRIX_ENABLE` is defined twice in `config.h` and isn't a current
  QMK option. It can be removed.

## Troubleshooting

- **`qmk: bad interpreter: .../python: no such file or directory`**: the `qmk`
  tool runs in its own Python environment, built against a specific Python
  version (see `/opt/homebrew/Cellar/qmk/*/libexec/pyvenv.cfg`). Homebrew
  removed that Python version when upgrading. `brew reinstall` doesn't help if
  the prebuilt package still targets the old version. Either install the
  missing version (e.g. `brew install python@3.13`), or rebuild `qmk` against
  the current Python with `brew reinstall --build-from-source qmk/qmk/qmk`.
- **`qmk flash` keeps waiting for the bootloader**: make sure the reset button
  is held *before* the cable is plugged in, that the cable goes straight to the
  half being flashed, and that the cable carries data.
- **Keys don't match the VIA layout after flashing**: expected. Reload
  `via.layout.json` (see [How firmware and VIA
  interact](#how-firmware-and-via-interact)).
- **Going back to stock**: flash with `-km default` (note that the upstream
  default keymap has no VIA support), or download Keychron's official firmware
  for the Q11 from their website and flash it the same way.
