#ifndef __WID_DISPLAY_CHARACTER_H
#define __WID_DISPLAY_CHARACTER_H

#include "widget.h"
#include "mgl.h"

#define DISPCH_CHAR_W 7
#define DISPCH_CHAR_H 9
#define DISPCH_GAP 2

typedef struct {
    Widget v;
    uint32_t color_pix;
    uint16_t fwaxs;
    uint16_t fways;
    uint16_t fwaxe;
    uint16_t fwaye;
    uint16_t fx;
    uint16_t fy;
    MglDisplay* disp;

    uint8_t cgram[64];
    char text[4][80];
    uint8_t chx;
    uint8_t chy;
} WidgetDisplayCh;

// please, use this macro to create a display object
// it will also create minimalgraphics api for internal use
// all you need in your app is a pointer to WidgetDisplayCh obejct and set of wDisplayCh.. methods from below
// ch_w, ch_h - width and height in characters (ex. 16, 2 - for 1602 lcd panel)
#define WID_DISPLAY_CHAR_DEFINE(inst_name, ch_w, ch_h)                                                                        \
    static WidgetDisplayCh inst_name;                                                                                         \
    static void inst_name##_SetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size) \
    {                                                                                                                         \
        inst_name.fwaxs = wax, inst_name.fways = way, inst_name.fwaxe = wax + wax_size, inst_name.fwaye = way + way_size;     \
        MGL_ASSERT(inst_name.fwaxs < (ch_w * DISPCH_CHAR_W + DISPCH_GAP));                                                    \
        MGL_ASSERT(inst_name.fwaxe <= (ch_w * DISPCH_CHAR_W + DISPCH_GAP));                                                   \
        MGL_ASSERT(inst_name.fways < (ch_h * DISPCH_CHAR_H + DISPCH_GAP));                                                    \
        MGL_ASSERT(inst_name.fwaye <= (ch_h * DISPCH_CHAR_H + DISPCH_GAP));                                                   \
        inst_name.fx = inst_name.fwaxs, inst_name.fy = inst_name.fways;                                                       \
    }                                                                                                                         \
    static void inst_name##_PixelOut(MglColor c)                                                                              \
    {                                                                                                                         \
        uint32_t* bmp = (uint32_t*)inst_name.v.surface->pixels;                                                               \
        bmp[(inst_name.fy + 1) * ((ch_w * DISPCH_CHAR_W + DISPCH_GAP) + 2) + inst_name.fx + 1] = c.wrd;                       \
        inst_name.fx++;                                                                                                       \
        if (inst_name.fx >= inst_name.fwaxe) {                                                                                \
            inst_name.fx = inst_name.fwaxs;                                                                                   \
            inst_name.fy++;                                                                                                   \
            if (inst_name.fy >= inst_name.fwaye) {                                                                            \
                inst_name.fy = inst_name.fways;                                                                               \
            }                                                                                                                 \
        }                                                                                                                     \
    }                                                                                                                         \
    static void inst_name##_Update() { ; }                                                                                    \
    static MglDispContext inst_name##_mglcontext;                                                                             \
    static MglDisplay inst_name##_mgldisp = {                                                                                 \
        .context = &inst_name##_mglcontext,                                                                                   \
        .size_x = (ch_w * DISPCH_CHAR_W + DISPCH_GAP),                                                                        \
        .size_y = (ch_h * DISPCH_CHAR_H + DISPCH_GAP),                                                                        \
        .setZone = inst_name##_SetZone,                                                                                       \
        .pixelOut = inst_name##_PixelOut,                                                                                     \
        .update = inst_name##_Update                                                                                          \
    };


void wDisplayChInit(
    WidgetDisplayCh* v,
    MglDisplay* disp,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend);

void wDisplayChSetCursor(WidgetDisplayCh* v, uint8_t x, uint8_t y);
void wDisplayChChar(WidgetDisplayCh* v, char ch);
void wDisplayChString(WidgetDisplayCh* v, char* str);
void wDisplayChCgram(WidgetDisplayCh* v, uint8_t addr, uint8_t value);

#endif // __WID_DISPLAY_CHARACTER_H