# Shortcuts for the common tasks. `make` checks the formatting, runs the tests
# and compiles the firmware. The firmware targets need the `qmk` tool (see
# README.md); the tests need a C compiler and a qmk_firmware checkout (for
# quantum/keycodes.h); the formatting targets need clang-format.

KEYBOARD := keychron/q11/iso_encoder
KEYMAP   := alphahydrae

KEYMAP_DIR := keyboards/$(KEYBOARD)/keymaps/$(KEYMAP)
BUILD_DIR  := test/build

# Where qmk_firmware is checked out (`qmk setup` clones it there).
QMK_HOME ?= $(HOME)/qmk_firmware

CC     ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -Werror -O1 -g
CFLAGS += -I$(KEYMAP_DIR) -I$(QMK_HOME)/quantum

CLANG_FORMAT ?= clang-format

# Every C file in the repository, as configured in .clang-format.
C_FILES := $(shell git ls-files '*.c' '*.h')

.PHONY: all format check-format test compile flash clean

all: check-format test compile

# Formats the C files in place.
format:
	$(CLANG_FORMAT) -i $(C_FILES)

# Fails if any C file isn't formatted, without changing it.
check-format:
	$(CLANG_FORMAT) --dry-run --Werror $(C_FILES)

# Builds and runs the unit tests (color logic, keymap fingerprint) with the
# computer's own C compiler (no keyboard, and only QMK's keycodes header).
test: $(BUILD_DIR)/test_lighting $(BUILD_DIR)/test_fingerprint
	./$(BUILD_DIR)/test_lighting
	./$(BUILD_DIR)/test_fingerprint

$(BUILD_DIR)/test_lighting: test/test_lighting.c test/check.h $(KEYMAP_DIR)/lighting.c $(KEYMAP_DIR)/lighting.h $(KEYMAP_DIR)/layers.h $(QMK_HOME)/quantum/keycodes.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ test/test_lighting.c $(KEYMAP_DIR)/lighting.c

$(BUILD_DIR)/test_fingerprint: test/test_fingerprint.c test/check.h $(KEYMAP_DIR)/fingerprint.c $(KEYMAP_DIR)/fingerprint.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ test/test_fingerprint.c $(KEYMAP_DIR)/fingerprint.c

compile:
	qmk compile -kb $(KEYBOARD) -km $(KEYMAP)

# Flashes one half: run it, then plug the half in with its reset button held
# (see "Flash (both halves)" in README.md).
flash:
	qmk flash -kb $(KEYBOARD) -km $(KEYMAP)

clean:
	rm -rf $(BUILD_DIR)
	rm -f *.bin *.hex *.uf2
