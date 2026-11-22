# Shortcuts for the common tasks. `make` runs the tests and compiles the
# firmware. The firmware targets need the `qmk` tool (see README.md); the tests
# only need a C compiler.

KEYBOARD := keychron/q11/iso_encoder
KEYMAP   := alphahydrae

KEYMAP_DIR := keyboards/$(KEYBOARD)/keymaps/$(KEYMAP)
BUILD_DIR  := test/build

CC     ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -Werror -O1 -g
CFLAGS += -I$(KEYMAP_DIR)

.PHONY: all test compile flash clean

all: test compile

# Builds and runs the unit tests of the color logic with the computer's own C
# compiler (no QMK, no keyboard).
test: $(BUILD_DIR)/test_lighting
	./$(BUILD_DIR)/test_lighting

$(BUILD_DIR)/test_lighting: test/test_lighting.c $(KEYMAP_DIR)/lighting.c $(KEYMAP_DIR)/lighting.h $(KEYMAP_DIR)/layers.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ test/test_lighting.c $(KEYMAP_DIR)/lighting.c

compile:
	qmk compile -kb $(KEYBOARD) -km $(KEYMAP)

# Flashes one half: run it, then plug the half in with its reset button held
# (see "Flash (both halves)" in README.md).
flash:
	qmk flash -kb $(KEYBOARD) -km $(KEYMAP)

clean:
	rm -rf $(BUILD_DIR)
	rm -f *.bin *.hex *.uf2
