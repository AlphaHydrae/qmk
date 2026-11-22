#pragma once

#define SPLIT_RGB_MATRIX_ENABLE
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LAYER_STATE_ENABLE
// Sync the Caps Lock state to the right half, for the Caps Lock overlay.
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_RGB_MATRIX_ENABLE

// Start on the per-layer color effect from rgb_matrix_user.inc. This only
// applies when the RGB settings in the EEPROM are reset.
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_layer_color
