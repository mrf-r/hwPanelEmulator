#ifndef __PANEL_H
#define __PANEL_H

#include "wid_button.h"
#include "wid_pot.h"
#include "wid_encoder.h"
#include "wid_framecounter.h"
#include "wid_display_character.h"
#include "wid_display_mono.h"
#include "wid_display_multi.h"

#define PANEL_NAME "LXR"
#define PANEL_SIZE_X 480
#define PANEL_SIZE_Y 320
#define PANEL_SCALE 2
#define PANEL_PROCESSRATEMUL 8
// #define FORM_COLOR_R 125
// #define FORM_COLOR_G 127
// #define FORM_COLOR_B 108
// #define FORM_COLOR_A 0
// #define FORM_COLOR_LABEL 0xFF000000

#define BUT_SIZE_X 46
#define BUT_SIZE_Y 46 // must be same as x
#define BUT_INTERVAL 50

// TODO: put all wids in one .h ?

void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t ms);

typedef struct {
    const char* name;
    const char* hint;
    const SDL_KeyCode keycode;
    const uint8_t midikey;
} ButtonDef;

static ButtonDef bdef[] = {

    { .name = "1", .hint = "a", .keycode = SDLK_a, .midikey = 0 },
    { .name = "2", .hint = "s", .keycode = SDLK_s, .midikey = 1 },
    { .name = "3", .hint = "d", .keycode = SDLK_d, .midikey = 2 },
    { .name = "4", .hint = "f", .keycode = SDLK_f, .midikey = 3 },
    { .name = "5", .hint = "g", .keycode = SDLK_g, .midikey = 4 },
    { .name = "6", .hint = "h", .keycode = SDLK_h, .midikey = 5 },
    { .name = "7", .hint = "j", .keycode = SDLK_j, .midikey = 6 },
    { .name = "8", .hint = "k", .keycode = SDLK_k, .midikey = 7 },
    { .name = "9", .hint = "z", .keycode = SDLK_z, .midikey = 8 },
    { .name = "10", .hint = "x", .keycode = SDLK_x, .midikey = 9 },
    { .name = "11", .hint = "c", .keycode = SDLK_c, .midikey = 10 },
    { .name = "12", .hint = "v", .keycode = SDLK_v, .midikey = 11 },
    { .name = "13", .hint = "b", .keycode = SDLK_b, .midikey = 12 },
    { .name = "14", .hint = "n", .keycode = SDLK_n, .midikey = 13 },
    { .name = "15", .hint = "m", .keycode = SDLK_m, .midikey = 14 },
    { .name = "16", .hint = ",", .keycode = SDLK_COMMA, .midikey = 15 },

    { .name = "Osc", .hint = "q", .keycode = SDLK_q, .midikey = 16 },
    { .name = "Aeg", .hint = "w", .keycode = SDLK_w, .midikey = 17 },
    { .name = "Mod", .hint = "e", .keycode = SDLK_e, .midikey = 18 },
    { .name = "Fm", .hint = "r", .keycode = SDLK_r, .midikey = 19 },
    { .name = "Clk", .hint = "t", .keycode = SDLK_t, .midikey = 20 },
    { .name = "Flt", .hint = "y", .keycode = SDLK_y, .midikey = 21 },
    { .name = "Lfo", .hint = "u", .keycode = SDLK_u, .midikey = 22 },
    { .name = "Mix", .hint = "i", .keycode = SDLK_i, .midikey = 23 },
};

__attribute__((unused)) static ButtonDef bdef_unused[] = {

    { .name = "v1_bd", .hint = "1", .keycode = SDLK_1, .midikey = 24 },
    { .name = "v2_bd", .hint = "2", .keycode = SDLK_2, .midikey = 25 },
    { .name = "v3_bd", .hint = "3", .keycode = SDLK_3, .midikey = 26 },
    { .name = "v4_sn", .hint = "4", .keycode = SDLK_4, .midikey = 27 },
    { .name = "v5_cm", .hint = "5", .keycode = SDLK_5, .midikey = 28 },
    { .name = "v6_hc", .hint = "6", .keycode = SDLK_6, .midikey = 29 },
    { .name = "v7_ho", .hint = "7", .keycode = SDLK_7, .midikey = 30 },

    { .name = "play", .hint = "space", .keycode = SDLK_SPACE, .midikey = 32 },
    { .name = "rec", .hint = "alt", .keycode = SDLK_LALT, .midikey = 33 },
    { .name = "shft", .hint = "", .keycode = SDLK_LSHIFT, .midikey = 34 },
    { .name = "copy", .hint = "ctrl", .keycode = SDLK_LCTRL, .midikey = 31 },
    { .name = "enc", .hint = "ret", .keycode = SDLK_RETURN, .midikey = 39 },

    { .name = "voice", .hint = "f1", .keycode = SDLK_F1, .midikey = 35 },
    { .name = "perf", .hint = "f2", .keycode = SDLK_F2, .midikey = 36 },
    { .name = "step", .hint = "f3", .keycode = SDLK_F3, .midikey = 37 },
    { .name = "menu", .hint = "f4", .keycode = SDLK_F4, .midikey = 38 },
};

static ButtonDef pot_def[] = {
    { .name = "1", .midikey = 1 },
    { .name = "2", .midikey = 2 },
    { .name = "3", .midikey = 3 },
    { .name = "4", .midikey = 4 },
};

#define BUTTONS_COUNT (sizeof(bdef) / sizeof(ButtonDef))
#define POTS_COUNT (sizeof(pot_def) / sizeof(ButtonDef))

static WidgetButton buttons[BUTTONS_COUNT];
static WidgetPot pots[POTS_COUNT];
static WidgetEnc encoder;
WID_DISPLAY_CHAR_DEFINE(display_ch, 16, 2) // static 1602 desplay
WID_DISPLAY_MONO_DEFINE(display_mono, 122, 32) // static monochrome display
WID_DISPLAY_MULTI_DEFINE(display_multi, 96, 64) // static monochrome display
static WidgetFrameCounter framecounter;

void panelConstruct(SDL_Renderer* rend)
{
    uint16_t xinit = 10 * PANEL_SCALE;
    uint16_t yinit = 170 * PANEL_SCALE;
    // uint16_t yinit = (PANEL_SIZE_Y - 10) * PANEL_SCALE;
    uint16_t x = xinit;
    uint16_t y = yinit;
    uint16_t pos = 0;
    for (; pos < 8; pos++) {
        wButtonInit(&buttons[pos], bdef[pos].name, bdef[pos].hint, bdef[pos].keycode, bdef[pos].midikey, x, y, rend);
        x += BUT_INTERVAL * PANEL_SCALE;
    }
    x = xinit;
    y += BUT_INTERVAL * PANEL_SCALE;
    for (; pos < 16; pos++) {
        wButtonInit(&buttons[pos], bdef[pos].name, bdef[pos].hint, bdef[pos].keycode, bdef[pos].midikey, x, y, rend);
        x += BUT_INTERVAL * PANEL_SCALE;
    }
    x = xinit;
    y += BUT_INTERVAL * PANEL_SCALE;
    for (; pos < 24; pos++) {
        wButtonInit(&buttons[pos], bdef[pos].name, bdef[pos].hint, bdef[pos].keycode, bdef[pos].midikey, x, y, rend);
        x += BUT_INTERVAL * PANEL_SCALE;
    }
    SDL_assert(pos == BUTTONS_COUNT);
    // potentiometers
    x = 200;
    y = 50;
    for (pos = 0; pos < POTS_COUNT; pos++) {
        // potInit(&pots[pos], pot_def[pos].name, pot_def[pos].midikey, x, y, rend);
        wPotInit(&pots[pos], pot_def[pos].name, pot_def[pos].midikey, x, y, rend);
        x += BUT_INTERVAL * PANEL_SCALE;
    }
    wEncInit(&encoder, "Enc!", "< >", SDLK_RIGHT, SDLK_LEFT, 0, x, y, rend);
    // display

    // wDisplayChInit(&display_ch, &display_ch_mgldisp, 200, 200, 4, rend);
    // wDisplayChSetCursor(&display_ch, 0, 0);
    // wDisplayChString(&display_ch, "0123456789ABCDEFG");
    // wDisplayChSetCursor(&display_ch, 2, 1);
    // wDisplayChString(&display_ch, "World");
    // wDisplayChCgram(&display_ch, 0, 0x1);
    // wDisplayChCgram(&display_ch, 1, 0x3);
    // wDisplayChCgram(&display_ch, 2, 0x7);
    // wDisplayChCgram(&display_ch, 3, 0xF);
    // wDisplayChCgram(&display_ch, 4, 0x1F);
    // wDisplayChCgram(&display_ch, 5, 0x1E);
    // wDisplayChCgram(&display_ch, 6, 0x1C);
    // wDisplayChCgram(&display_ch, 7, 0x18);
    // wDisplayChCgram(&display_ch, 8, 0x10);
    // wDisplayMonoInit(&display_mono, &display_mono_mgldisp, display_mono_framebuffer, 200, 200, 4, rend);
    wDisplayMonoInit(&display_mono, &display_mono_mgldisp, display_mono_framebuffer, 200, 200, 3, rend);
    mgsDisplay(&display_mono_mgldisp);
    mgdString("Hello!", COLOR_ON);
    // display_mono.color_pix = 0xFFFFFF;
    widgetLed(&display_mono.v, 0xFF800000);
    display_mono.color_pix = 0;
    // display_mono.v.need_redraw = 1;
    

    // wDisplayMultiInit(&display_multi, &display_multi_mgldisp, 100, 200, 2, rend);
    // mgsDisplay(&display_multi_mgldisp);
    // MglColor back = { .wrd = 0xFF000000 };
    // mgdFill(back);
    // mgdHsvTestFill();
    // mgdHsvTestFill2();
    // mgdString("Hello!", mgColorHsv(0, 255, 128));

    wFrameCounterInit(&framecounter, 5, 5, rend);
    // return;
}

void panelLoop(uint32_t ms)
{
    //
}

void wButtonMidiSend(uint8_t midictrl, uint8_t value)
{
    if (value >= 64) {
        if (midictrl == 0) {
            wDisplayChSetCursor(&display_ch, 0, 0);
        }
        wDisplayChChar(&display_ch, midictrl + 'A');
    }
    // display_ch_SetZone(midictrl, 3, 10, 10);
    // MglColor p = { .wrd = 0xFFFF0000 };
    // display_ch_PixelOut(p);
    // display_ch.v.need_redraw = 1;
    // (void)midictrl;
    // (void)value;
}
#endif // __PANEL_H