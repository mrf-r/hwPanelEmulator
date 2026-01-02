
#include "wid_display_character.h"
#include "wid_graphics.h"
#include "panel_conf.h"

// display
// #define DISPCH_COLOR_BACK 0x000000FF // blue inverted
// #define DISPCH_COLOR_PIXEL 0xFFFFFF
#ifndef DISPCH_COLOR_BACK
#define DISPCH_COLOR_BACK 0x0040C000 // green standart
#endif
#ifndef DISPCH_COLOR_PIXEL
#define DISPCH_COLOR_PIXEL 0x0
#endif
#define DISPCH_ALPHA 0xA0
extern const MglFont _5x7mod;

static void wDispChRedraw(void* wid)
{
    WidgetDisplayCh* v = (WidgetDisplayCh*)wid;
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

static WidgetApi wDispChApi = {
    .redraw = wDispChRedraw,
    .process = 0,
    .keyboard = 0,
    .touchMove = 0,
    .touchClick = 0,
    .mouseWheel = 0,
    .terminate = 0
};

void wDisplayChInit(
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
}

void wDisplayChSetCursor(WidgetDisplayCh* v, uint8_t x, uint8_t y)
{
    v->chx = x % 80;
    v->chy = y % 4;
}
void wDisplayChChar(WidgetDisplayCh* v, char ch)
{
    v->text[v->chy][v->chx] = ch;
    v->chx++;
    if (v->chx == 80)
        v->chx = 0;
    v->v.need_redraw = 1;
}
void wDisplayChString(WidgetDisplayCh* v, char* str)
{
    char* ch = str;
    while (*ch) {
        wDisplayChChar(v, *ch);
        ch++;
    }
}
void wDisplayChCgram(WidgetDisplayCh* v, uint8_t addr, uint8_t value)
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
