# Plan

Remaining work on the Keychron Q11 firmware described in
[README.md](./README.md).

- [Migrating an install from the symlink setup](#migrating-an-install-from-the-symlink-setup)
- [To verify against the QMK source](#to-verify-against-the-qmk-source)

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
