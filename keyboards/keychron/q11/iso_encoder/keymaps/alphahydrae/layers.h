#pragma once

// Layers 0 and 2 are the two default layers selected by the Mac/Windows switch
// (Mac = 0, Windows = 2, see q11.c in QMK). TG(BRACKETS) also toggles layer 2
// on top of layer 0.
enum layers {
    BASE,
    FN,
    BRACKETS,
    BRACKETS_FN,
};
