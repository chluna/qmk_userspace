// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#include QMK_KEYBOARD_H

#include "manna-harbour_miryoku.h"

#include "os_detection.h"

#include "features/custom_shift_keys.h"
#include "features/casemodes.h"
#include "features/flow.h"


// Additional Features double tap guard

enum {
    U_TD_BOOT,
#define MIRYOKU_X(LAYER, STRING) U_TD_U_##LAYER,
MIRYOKU_LAYER_LIST
#undef MIRYOKU_X
    TD_RGB_MOD,
};

void u_td_fn_boot(tap_dance_state_t *state, void *user_data) {
  if (state->count == 2) {
    reset_keyboard();
  }
}

#define MIRYOKU_X(LAYER, STRING) \
void u_td_fn_U_##LAYER(tap_dance_state_t *state, void *user_data) { \
  if (state->count == 2) { \
    default_layer_set((layer_state_t)1 << U_##LAYER); \
  } \
}
MIRYOKU_LAYER_LIST
#undef MIRYOKU_X

#if defined (RGB_MATRIX_ENABLE) || defined (RGBLIGHT_ENABLE)
void td_rgb_modes(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
#           if defined (RGB_MATRIX_ENABLE)
                rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
#           endif
#           if defined (RGBLIGHT_ENABLE)
                rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
#           endif
            break;
        case 2:
#           if defined (RGB_MATRIX_ENABLE)
                rgb_matrix_mode(RGB_MATRIX_BREATHING);
#           endif
#           if defined (RGBLIGHT_ENABLE)
                rgblight_mode(RGBLIGHT_MODE_BREATHING);
#           endif
            break;
        case 3:
#           if defined (RGB_MATRIX_ENABLE)
                rgb_matrix_mode(RGB_MATRIX_CYCLE_LEFT_RIGHT);
#           endif
#           if defined (RGBLIGHT_ENABLE)
                rgblight_mode(RGBLIGHT_MODE_RAINBOW_MOOD);
#           endif
            break;
        case 4:
#           if defined (RGB_MATRIX_ENABLE)
                rgb_matrix_mode(RGB_MATRIX_CYCLE_PINWHEEL);
#           endif
#           if defined (RGBLIGHT_ENABLE)
                rgblight_mode(RGBLIGHT_MODE_RAINBOW_SWIRL);
#           endif
            break;
    }
}
#endif

tap_dance_action_t tap_dance_actions[] = {
    [U_TD_BOOT] = ACTION_TAP_DANCE_FN(u_td_fn_boot),
#define MIRYOKU_X(LAYER, STRING) [U_TD_U_##LAYER] = ACTION_TAP_DANCE_FN(u_td_fn_U_##LAYER),
    MIRYOKU_LAYER_LIST
#undef MIRYOKU_X
#if defined (RGB_MATRIX_ENABLE) || defined (RGBLIGHT_ENABLE)
    [TD_RGB_MOD] = ACTION_TAP_DANCE_FN(td_rgb_modes),
#endif
};


// Custom macros

enum custom_keycodes {
    O_APP = SAFE_RANGE,
    XC_CUSTOM,
    XC_KC_UNDS,
    XC_KC_MINS,
    XC_KC_SLSH,
    XC_KC_BSLS,
};


// Custom functions

void invoke_app_launcher(void) {
    os_variant_t os = detected_host_os();
    switch (os) {
        case OS_LINUX:
            tap_code16(LGUI(KC_SPC));
            break;
        case OS_WINDOWS:
            tap_code16(LALT(KC_SPC));
            break;
        case OS_MACOS:
        case OS_IOS:
            tap_code16(LCMD(KC_SPC));
            break;
        default:
            // send nothing
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!update_flow(keycode, record)) {
        return false;
    }

    if (!process_custom_shift_keys(keycode, record)) {
        return false;
    }

    if (!process_case_modes(keycode, record)) {
        return false;
    }

    if (record->event.pressed) {
        switch (keycode) {
            case O_APP:
                invoke_app_launcher();
                break;
            case XC_CUSTOM:
                enable_xcase();
                break;
            case XC_KC_UNDS:
                enable_xcase_with(KC_UNDS);
                break;
            case XC_KC_MINS:
                enable_xcase_with(KC_MINS);
                break;
            case XC_KC_SLSH:
                enable_xcase_with(KC_SLSH);
                break;
            case XC_KC_BSLS:
                enable_xcase_with(KC_BSLS);
                break;
        }
    }

    return true;
};

bool use_default_xcase_separator(uint16_t keycode, const keyrecord_t *record) {
    switch (keycode) {
        case KC_A ... KC_Z:
        case KC_1 ... KC_0:
            return true;
    }
    return false;
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TD(U_TD_U_BASE):
        case TD(U_TD_U_EXTRA):
        case TD(U_TD_U_TAP):
        case TD(U_TD_U_NUM):
        case TD(U_TD_U_NAV):
        case TD(U_TD_BOOT):
            return 300;
        case TD(TD_RGB_MOD):
            return 500;
        default:
            return g_tapping_term;
    }
}


// Keymap

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
#define MIRYOKU_X(LAYER, STRING) [U_##LAYER] = U_MACRO_VA_ARGS(MIRYOKU_LAYERMAPPING_##LAYER, MIRYOKU_LAYER_##LAYER),
    MIRYOKU_LAYER_LIST
#undef MIRYOKU_X
};


// Shift functions

const custom_shift_key_t custom_shift_keys[] = {
    {KC_COMM, KC_SCLN},
    {KC_DOT,  KC_COLN},
    {KC_SLSH, KC_BSLS},
    {KC_VOLU, KC_BRIU},
    {KC_VOLD, KC_BRID},
    {CW_TOGG, KC_CAPS}
};
uint8_t NUM_CUSTOM_SHIFT_KEYS = sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);


// Dalius-style one shot keys

const uint16_t flow_config[FLOW_COUNT][2] = {
    {LT(U_NAV,KC_SPC),   KC_LSFT},
    {LT(U_NAV,KC_SPC),   KC_LCTL},
    {LT(U_NAV,KC_SPC),   KC_LALT},
    {LT(U_NAV,KC_SPC),   KC_LGUI},
    {LT(U_NUM,KC_BSPC),  KC_LSFT},
    {LT(U_NUM,KC_BSPC),  KC_LCTL},
    {LT(U_NUM,KC_BSPC),  KC_LALT},
    {LT(U_NUM,KC_BSPC),  KC_LGUI},
    {LT(U_MOUSE,KC_TAB), KC_LSFT},
    {LT(U_MOUSE,KC_TAB), KC_LCTL},
    {LT(U_MOUSE,KC_TAB), KC_LALT},
    {LT(U_MOUSE,KC_TAB), KC_LGUI},
    {LT(U_SYM,KC_ENT),   KC_LSFT},
    {LT(U_SYM,KC_ENT),   KC_LCTL},
    {LT(U_SYM,KC_ENT),   KC_LALT},
    {LT(U_SYM,KC_ENT),   KC_LGUI},
};

void matrix_scan_user(void) {
    flow_matrix_scan();
}


// Combos

const uint16_t PROGMEM app_launch_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_G, COMBO_END};
const uint16_t PROGMEM app_launch_r_combo[] = {LT(U_SYM, KC_ENT), KC_M, COMBO_END};
const uint16_t PROGMEM dyn_macro_ply_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_F, KC_P, COMBO_END};
const uint16_t PROGMEM dyn_macro_rec_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_W, KC_F, COMBO_END};
const uint16_t PROGMEM dyn_macro_stp_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_Q, KC_W, COMBO_END};
const uint16_t PROGMEM dyn_macro_ply_r_combo[] = {LT(U_SYM, KC_ENT), KC_U, KC_L, COMBO_END};
const uint16_t PROGMEM dyn_macro_rec_r_combo[] = {LT(U_SYM, KC_ENT), KC_Y, KC_U, COMBO_END};
const uint16_t PROGMEM dyn_macro_stp_r_combo[] = {LT(U_SYM, KC_ENT), KC_QUOT, KC_Y, COMBO_END};
const uint16_t PROGMEM und_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_Z, COMBO_END};
const uint16_t PROGMEM rdo_l_combo[] = {LT(U_MOUSE, KC_TAB), KC_X, COMBO_END};
const uint16_t PROGMEM cut_l_combo[] = {KC_Z, KC_X, COMBO_END};
const uint16_t PROGMEM cpy_l_combo[] = {KC_X, KC_C, COMBO_END};
const uint16_t PROGMEM pst_l_combo[] = {KC_C, KC_D, COMBO_END};
const uint16_t PROGMEM und_r_combo[] = {LT(U_SYM, KC_ENT), KC_SLSH, COMBO_END};
const uint16_t PROGMEM rdo_r_combo[] = {LT(U_SYM, KC_ENT), KC_DOT, COMBO_END};
const uint16_t PROGMEM cut_r_combo[] = {KC_SLSH, KC_DOT, COMBO_END};
const uint16_t PROGMEM cpy_r_combo[] = {KC_DOT, KC_COMM, COMBO_END};
const uint16_t PROGMEM pst_r_combo[] = {KC_COMM, KC_H, COMBO_END};

combo_t key_combos[] = {
    COMBO(app_launch_l_combo,    O_APP),
    COMBO(app_launch_r_combo,    O_APP),
    COMBO(dyn_macro_ply_l_combo, DM_PLY1),
    COMBO(dyn_macro_rec_l_combo, DM_REC1),
    COMBO(dyn_macro_stp_l_combo, DM_RSTP),
    COMBO(dyn_macro_ply_r_combo, DM_PLY2),
    COMBO(dyn_macro_rec_r_combo, DM_REC2),
    COMBO(dyn_macro_stp_r_combo, DM_RSTP),
    COMBO(und_l_combo,           U_UND),
    COMBO(rdo_l_combo,           U_RDO),
    COMBO(cut_l_combo,           U_CUT),
    COMBO(cpy_l_combo,           U_CPY),
    COMBO(pst_l_combo,           U_PST),
    COMBO(und_r_combo,           U_UND),
    COMBO(rdo_r_combo,           U_RDO),
    COMBO(cut_r_combo,           U_CUT),
    COMBO(cpy_r_combo,           U_CPY),
    COMBO(pst_r_combo,           U_PST),
};

uint8_t combo_ref_from_layer(uint8_t layer) {
    switch (layer) {
        case U_EXTRA:
            return U_BASE;
    }
    return layer;
}

// OLED

#if defined (OLED_ENABLE)
oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    } else {
        return OLED_ROTATION_180;
    }
    return rotation;
}

static void render_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,240,240,128,240,240,128,240,240,128,240,240,128,240,240,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,182,182,182,255,255,255,  3,  3,255,255,255,  3,  3,255,255,255,  3,  3,255,255,255,182,182,182,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,240,112, 48, 48, 48,112,224,224,128,  0,  0,255,255,255, 96,112, 48, 48,112,240,240,224,  0,  0,  0,  0,255,255,255,  0,  0,  0,240,240,  0,  0,  0,  0,  0,  0,240,240,240,  0,  0,  0,240,240,240, 96, 48, 48, 48,112,240,224,192,  0,  0,  0, 64,224,240,112, 48, 48, 48,112,240,224,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,109,109,109,255,255,255,254,254,248,241,243,192,192,243,241,248,252,254,255,255,255,109,109,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 63,127,255,224,192,128,128,128,192,224,224, 32,  0,  0,255,255,255,  0,  0,  0,  0,  0,  0,255,255,254,  0,  0,  0,255,255,255,  0,  0,  0,127,255,240,192,128,128,192,224,255,255,255,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,248,252,220,142,142,134,134,198,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1, 15, 15,  1, 15, 15,  1, 15, 15,  1, 15, 15,  1, 15, 15,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_linux_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,192,224,112, 56, 24, 12, 12, 12, 12, 12, 12, 24, 56,112,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,  0,  0, 56, 56, 56,  0,  0,  0,  0, 56, 56, 56,  0,  0,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,224,254, 31,  0,  0,  0,  3,131,134,198,204,204,198,134,131,  3,  0,  0,  0, 31,254,224,128,  0,  0,  0,  0,  0,  0,  0,  6,  7,  3,  0,  0,  0,  0,  6,  7,  3,  1,  0,  0,  0,  0,  1,  3,  7,  6,  0,  0,  0,  0,  3,  7,  6,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_windows_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,128,128,128,128,128,128,  0,  0,192,192, 64, 96, 96, 96, 96, 96, 32, 48,240,240,  0,  0,  0,  0,  0,  0,  0,  0,127,127, 99, 97, 97, 97, 97, 97,127,127,  0,  0,127,127, 96, 96, 96, 96, 96, 96, 96, 96,127,127,  0,  0,  0,  0,  0,  0,  0,  0,254,254,198,134,134,134,134,134,254,254,  0,  0,254,254,  6,  6,  6,  6,  6,  6,  6,  6,254,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  0,  0,  3,  3,  2,  6,  6,  6,  6,  6,  4, 12, 15, 15,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_apple_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,128,128,192,192,192,192,192,192,128,128,152,158,198,195,195,195,192,192,128,128,  0,  0,  0,  0,  0,  0,  0,  0,240,252, 30,  7,  3,  1,  0,  0,  0,  0,  0,  0,  1,  3,  3,  1,  0,  0,  0,  0,  0,224,249, 31,  7,  3,  0,  0,  0,  0,  0,  0,  7, 63,248,224,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3, 15,252,240, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7, 14, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 14,  7,  3,  1,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_caps_lock(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,224,224, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,224,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,128,192,224,240,120, 60, 60,120,240,224,192,128,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0, 49, 51, 49, 48, 48, 48, 48, 48, 48, 49, 51, 49,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_blank(void) {
    static const char PROGMEM raw_logo[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_base(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,128,128,192,192, 96,112, 48, 56, 24, 12, 12, 24, 56, 48,112, 96,192,192,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,134,134, 15, 31, 25, 57, 48, 96,224,192,192,128,  0,  0,128,192,192,224, 96, 48, 57, 25, 31, 15,134,134,  0,  0,  0,  0,  0,  0, 97, 97,195,199,134,142, 12, 24, 56, 48,113, 97,195,195, 97,113, 48, 56, 24, 12,142,134,199,195, 97, 97,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  3,  6, 14, 12, 28, 24, 48, 48, 24, 28, 12, 14,  6,  3,  3,  1,  1,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_extra(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,128,128,192,192, 96,112, 48, 56, 24, 12, 12, 24, 56, 48,112, 96,192,192,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,134,134, 15, 31, 25, 57, 48, 96,224,192,192,128,  0,  0,128,192,192,224, 96, 48, 57, 25, 31, 15,134,134,  0,  0,  0,  0,  0,  0, 97, 97,195,199,134,142, 12, 24, 56, 48,113, 97,195,195, 97,113, 48, 56, 24, 12, 14,  6,231,227,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  3,  6, 14, 12, 28, 24, 48, 48, 24, 24,  0,  0,  0,  3,  3,  3, 31, 31,  3,  3,  3,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_tap(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,192,224, 96, 48, 48, 96,224,192,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 48, 48,120,252,204,206,134,  3,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  3,134,206,204,252,120, 48, 48,  0,  0,  6,  6, 12, 28, 24, 57, 49, 99, 99,198,206,140,156, 24, 48, 48, 24,156,140,206,198, 99, 99, 49, 57, 24, 28, 12,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  3,  6,  6,  3,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_button(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,128,192,224, 96,112, 48, 48, 24, 24, 24, 24, 24, 24, 48, 48,112, 96,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,224,252, 31,  7,  1,  0,192,240, 56, 12, 12,  6,  6,  6,  6, 12, 12, 56,240,192,  0,  1,  7, 31,252,224,  0,  0,  0,  0,  0,  0,  7, 63,248,224,128,  0,  3, 15, 28, 48, 48, 96, 96, 96, 96, 48, 48, 28, 15,  3,  0,128,224,248, 63,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  6, 14, 12, 12, 24, 24, 24, 24, 24, 24, 12, 12, 14,  6,  7,  3,  1,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_nav(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,192,224, 96,224,192,192,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  7, 63,248,192,  0,  1,  1,  1,  3,  3,  7,  6,  6, 12, 12, 12, 24, 24, 24, 48, 48,112,224,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  7, 63,248,192,  0,  0,  0,  0,224,252, 28, 12,  6,  6,  6,  3,  3,  3,  3,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1, 15, 62,120,112,127, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_mouse(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,224,240, 48, 48, 96, 96, 96,192,192,192,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7, 63,248,192,  0,  0,  0,  0,  0,  1,  1,  1,  3,  3,131,134,198,198,204,108,108, 56, 56,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1, 15,126,240,128,  0,  0,224,248, 24, 56,112,227,199,142, 28, 56,112,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3, 15, 12, 15,  3,  0,  0,  0,  0,  0,  1,  3,  7, 14, 12, 14,  7,  3,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_media(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,192,192,224,224,192,192,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,  1,  0,  0,  0,  1,  1,  3,  7,  7, 14, 12, 28, 24, 56,112,240,224,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,128,  0,  0,  0,128,128,192,224,224,112, 48, 56, 24, 28, 14, 15,  7,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  7,  7,  3,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_num(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,240,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  3,  1,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_sym(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 96,224,192,128,128,192,224, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0,  0,  0,  0, 24, 28, 15,  7,  7, 15, 28, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,192,192,192,252,252,192,192,192,192,  0,  0,  0,  0, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_fun(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,224,112, 48, 48, 48, 48, 48, 48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,128,128,128,128,128,192,255,159,128,128,128,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,249,255,  3,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 14,  7,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_layer_undef(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,192,224, 96, 96, 96, 96, 96,  0,  0,  0,  0,  0,  0,  0,  0, 96, 96, 96, 96, 96,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0,  0,  0, 12, 14, 14,  6,134,198,254,124,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,240,240,  0,  0,  0,  0,  0,  0,  0,  0,103,119,  1,  0,  0,  0,  0,  0,  0,  0,240,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  7,  6,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  7,  3,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_ctrl(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_shft(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_alt(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_gui(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_cs(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_ca(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_cg(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_sa(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_sg(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_ag(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_csa(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_csg(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_cag(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_sag(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

static void render_mod_csag(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,128,192,224,224,192,128,  0,  0,  0,  0,  0,  0,192,224,240,184, 28, 14,  6,  6, 14, 28,184,240,224,192,  0,  0,  0,  0, 14,  7,  3,  1,  0,  0,  1,  3,  7, 14,  4,  0,  0,  0,  1,  1,  1, 31, 31, 48, 48, 48, 48, 31, 31,  1,  1,  1,  0,  0,  0, 24, 28,126,220,152,  0,  0,152,252,254, 28, 24,  0,  0,  0,128,192,224,224,216,188,126,126,188,216,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  3,127,127,  3,  1,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7, 27, 61,126,126, 61, 27,  7,  7,  3,  1,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

void render_detected_os(void) {
    os_variant_t os = detected_host_os();
    switch (os) {
        case OS_LINUX:
            render_linux_logo();
            break;
        case OS_WINDOWS:
            render_windows_logo();
            break;
        case OS_MACOS:
        case OS_IOS:
            render_apple_logo();
            break;
        default:
            render_blank();
            break;
    }
}

void render_layer_state(void) {
    switch (get_highest_layer(layer_state)) {
        case U_BASE:
            switch (get_highest_layer(default_layer_state)) {
                case U_BASE:
                    render_layer_base();
                    break;
                case U_EXTRA:
                    render_layer_extra();
                    break;
                case U_TAP:
                    render_layer_tap();
                    break;
            }
            break;
        case U_BUTTON:
            render_layer_button();
            break;
        case U_NAV:
            render_layer_nav();
            break;
        case U_MOUSE:
            render_layer_mouse();
            break;
        case U_MEDIA:
            render_layer_media();
            break;
        case U_NUM:
            render_layer_num();
            break;
        case U_SYM:
            render_layer_sym();
            break;
        case U_FUN:
            render_layer_fun();
            break;
        default:
            render_layer_undef();
            break;
    }
}

void render_mod_state(void) {
    uint8_t mods = get_mods();
    bool is_mod_ctrl = mods & MOD_MASK_CTRL;
    bool is_mod_shft = mods & MOD_MASK_SHIFT;
    bool is_mod_alt = mods & MOD_MASK_ALT;
    bool is_mod_gui = mods & MOD_MASK_GUI;

    if (is_mod_ctrl && !is_mod_shft && !is_mod_alt && !is_mod_gui) {
        render_mod_ctrl();
    } else if (!is_mod_ctrl && is_mod_shft && !is_mod_alt && !is_mod_gui) {
        render_mod_shft();
    } else if (!is_mod_ctrl && !is_mod_shft && is_mod_alt && !is_mod_gui) {
        render_mod_alt();
    } else if (!is_mod_ctrl && !is_mod_shft && !is_mod_alt && is_mod_gui) {
        render_mod_gui();
    } else if (is_mod_ctrl && is_mod_shft && !is_mod_alt && !is_mod_gui) {
        render_mod_cs();
    } else if (is_mod_ctrl && !is_mod_shft && is_mod_alt && !is_mod_gui) {
        render_mod_ca();
    } else if (is_mod_ctrl && !is_mod_shft && !is_mod_alt && is_mod_gui) {
        render_mod_cg();
    } else if (!is_mod_ctrl && is_mod_shft && is_mod_alt && !is_mod_gui) {
        render_mod_sa();
    } else if (!is_mod_ctrl && is_mod_shft && !is_mod_alt && is_mod_gui) {
        render_mod_sg();
    } else if (!is_mod_ctrl && !is_mod_shft && is_mod_alt && is_mod_gui) {
        render_mod_ag();
    } else if (is_mod_ctrl && is_mod_shft && is_mod_alt && !is_mod_gui) {
        render_mod_csa();
    } else if (is_mod_ctrl && is_mod_shft && !is_mod_alt && is_mod_gui) {
        render_mod_csg();
    } else if (is_mod_ctrl && !is_mod_shft && is_mod_alt && is_mod_gui) {
        render_mod_cag();
    } else if (!is_mod_ctrl && is_mod_shft && is_mod_alt && is_mod_gui) {
        render_mod_sag();
    } else if (is_mod_ctrl && is_mod_shft && is_mod_alt && is_mod_gui) {
        render_mod_csag();
    } else {
        render_blank();
    }
}

void render_kb_state(void) {
    render_detected_os();
    oled_set_cursor(0, 4);
    render_layer_state();
    oled_set_cursor(0, 8);
    render_mod_state();
    oled_set_cursor(0, 12);
    if (host_keyboard_led_state().caps_lock) {
        render_caps_lock();
    } else {
        render_blank();
    }
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (is_keyboard_master()) {
        render_kb_state();
    } else {
        render_logo();
        oled_scroll_left();
    }
    return false;
}
#endif


// Thumb combos

#if defined (MIRYOKU_KLUDGE_THUMBCOMBOS)
const uint16_t PROGMEM thumbcombos_base_right[] = {LT(U_SYM, KC_ENT), LT(U_NUM, KC_BSPC), COMBO_END};
const uint16_t PROGMEM thumbcombos_base_left[] = {LT(U_NAV, KC_SPC), LT(U_MOUSE, KC_TAB), COMBO_END};
const uint16_t PROGMEM thumbcombos_nav[] = {KC_ENT, KC_BSPC, COMBO_END};
const uint16_t PROGMEM thumbcombos_mouse[] = {KC_BTN2, KC_BTN1, COMBO_END};
const uint16_t PROGMEM thumbcombos_media[] = {KC_MSTP, KC_MPLY, COMBO_END};
const uint16_t PROGMEM thumbcombos_num[] = {KC_0, KC_MINS, COMBO_END};
  #if defined (MIRYOKU_LAYERS_FLIP)
const uint16_t PROGMEM thumbcombos_sym[] = {KC_UNDS, KC_LPRN, COMBO_END};
  #else
const uint16_t PROGMEM thumbcombos_sym[] = {KC_RPRN, KC_UNDS, COMBO_END};
  #endif
const uint16_t PROGMEM thumbcombos_fun[] = {KC_SPC, KC_TAB, COMBO_END};
combo_t key_combos[] = {
  COMBO(thumbcombos_base_right, LT(U_FUN, KC_DEL)),
  COMBO(thumbcombos_base_left, LT(U_MEDIA, KC_ESC)),
  COMBO(thumbcombos_nav, KC_DEL),
  COMBO(thumbcombos_mouse, KC_BTN3),
  COMBO(thumbcombos_media, KC_MUTE),
  COMBO(thumbcombos_num, KC_DOT),
  #if defined (MIRYOKU_LAYERS_FLIP)
  COMBO(thumbcombos_sym, KC_RPRN),
  #else
  COMBO(thumbcombos_sym, KC_LPRN),
  #endif
  COMBO(thumbcombos_fun, KC_APP)
};
#endif
