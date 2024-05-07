#ifndef __WID_DISPLAY_CHARACTER_H
#define __WID_DISPLAY_CHARACTER_H

#include "mgl.h"
#include "wid_graphics.h"
#include "widget.h"

// display
// #define DISPCH_COLOR_BACK 0x000000FF // blue inverted
// #define DISPCH_COLOR_PIXEL 0xFFFFFF
#define DISPCH_COLOR_BACK 0x0040C000 // green standart
#define DISPCH_COLOR_PIXEL 0x0
#define DISPCH_ALPHA 0xA0
#define DISPCH_CHAR_W 7
#define DISPCH_CHAR_H 9
#define DISPCH_GAP 2
extern const MglFont _5x7mod;

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

static void wDispChRedraw(void* wid)
{
    WidgetDisplayCh* v = (WidgetDisplayCh*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        drawOutline(&v->v, panel.widget_color_released);

        mgsDisplay(v->disp);

        mgsFont(&_5x7mod);
        MglColor led = { .wrd = (v->v.led & 0x00FFFFFF) | (DISPCH_ALPHA << 24) };
        MglColor pix = { .wrd = (v->color_pix & 0x00FFFFFF) | (DISPCH_ALPHA << 24) };
        mgsBackColor(led);
        mgdFill(led);
        pix = mgAlphablend(0x60, pix, led);
        pix.alpha = DISPCH_ALPHA;
        for (uint8_t y = 0; y < 4; y++) {
            for (uint8_t x = 0; x < 80; x++) {
                mgsCursorAbs(x * DISPCH_CHAR_W + DISPCH_GAP, y * DISPCH_CHAR_H + DISPCH_GAP);
                char c = v->text[y][x];
                if (c >= 0x20) {
                    if ((signed char)c < 0)
                        c = 127;
                    mgdChar(c, pix);
                } else if (c < 8) {
                    mgdBitmap(&v->cgram[c * 8], 5, 7, 7, pix);
                    ;
                }
            }
        }
    }
}

static WidgetApi wDispChApi = {
    .redraw = wDispChRedraw,
    .process = 0,
    .keyboard = 0,
    .mouseMove = 0,
    .mouseClick = 0,
    .mouseWheel = 0
};

__attribute__((unused)) static void wDisplayChInit(
    WidgetDisplayCh* v,
    MglDisplay* disp,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend)
{
    SDL_memset(disp->context, 0, sizeof(MglDispContext));
    SDL_memset(v, 0, sizeof(WidgetDisplayCh));
    widgetInit(&v->v, (void*)v, &wDispChApi, x, y, disp->size_x + 2, disp->size_y + 2, scale, rend);
    v->disp = disp;
    v->color_pix = DISPCH_COLOR_PIXEL;
    widgetLed(&v->v, DISPCH_COLOR_BACK);

    // mgsDisplay(disp);
    // mgsFont(&_5x7mod);
    // MglColor led = { .wrd = v->v.led };
    // MglColor pix = { .wrd = v->color_pix };
    // MglColor led2 = { .wrd = v->v.led | 0xFF };
    // mgsBackColor(led2);
    // MglColor tc = { .wrd = 0xFFFF0000 };
    // mgdFill(led);
    // mgsCursorAbs(0 * DISPCH_CHAR_W + DISPCH_CHAR_W / 2, 0 * DISPCH_CHAR_H + DISPCH_CHAR_W / 2);
    // mgdChar('I', pix);
}

// ========================================================================================
// midi api char

__attribute__((unused)) static void wDisplayChSetCursor(WidgetDisplayCh* v, uint8_t x, uint8_t y)
{
    v->chx = x % 80;
    v->chy = y % 4;
}
__attribute__((unused)) static inline void wDisplayChChar(WidgetDisplayCh* v, char ch)
{
    v->text[v->chy][v->chx] = ch;
    v->chx++;
    if (v->chx == 80)
        v->chx = 0;
    v->v.need_redraw = 1;
}
__attribute__((unused)) static void wDisplayChString(WidgetDisplayCh* v, char* str)
{
    char* ch = str;
    while (*ch) {
        wDisplayChChar(v, *ch);
        ch++;
    }
}
__attribute__((unused)) static void wDisplayChCgram(WidgetDisplayCh* v, uint8_t addr, uint8_t value)
{
    uint8_t rev = 0;
    for (int i = 0; i < 5; i++) {
        rev |= ((value >> i) & 0x1) << (4 - i);
    }
    if (addr < 64) {
        v->cgram[addr] = rev;
    }
    v->v.need_redraw = 1;
}

#endif // __WID_DISPLAY_CHARACTER_H