#ifndef __WID_GRAPHICS_H
#define __WID_GRAPHICS_H

#include "widget.h"
// Generated from Convert.js
// Font made by mrf in SG Bitmap Font Editor http://armag.hut1.ru/sgfed.htm
// Conversion script by mrf
// SymbolsCount = 128, Width = 8, Height = 8

static const uint8_t fnt_6_blcd[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*   */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0xc0, 0x00, /* ! */
    0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* " */
    0x00, 0x24, 0xfe, 0x6c, 0xfe, 0x48, 0x00, 0x00, /* # */
    0x10, 0x7c, 0xe0, 0x78, 0x1c, 0xf8, 0x20, 0x00, /* $ */
    0xe4, 0xac, 0xf8, 0x30, 0x7c, 0xd4, 0x9c, 0x00, /* % */
    0x60, 0xd0, 0xd6, 0x7c, 0xc8, 0xcc, 0x76, 0x00, /* & */
    0x40, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, /* ' */
    0x60, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x00, /* ( */
    0xc0, 0x60, 0x60, 0x60, 0x60, 0x60, 0xc0, 0x00, /* ) */
    0x00, 0x20, 0xa8, 0x70, 0xa8, 0x20, 0x00, 0x00, /* * */
    0x00, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x00, 0x00, /* + */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xc0, 0x80, /* , */
    0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, /* - */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0x00, /* . */
    0x30, 0x30, 0x60, 0x60, 0x60, 0xc0, 0xc0, 0x00, /* / */
    0x78, 0xcc, 0xdc, 0xfc, 0xec, 0xcc, 0x78, 0x00, /* 0 */
    0x30, 0x70, 0xb0, 0x30, 0x30, 0x30, 0xfc, 0x00, /* 1 */
    0xf8, 0x0c, 0x0c, 0x78, 0xc0, 0xc0, 0xfc, 0x00, /* 2 */
    0xfc, 0x18, 0x30, 0x78, 0x0c, 0x0c, 0xf8, 0x00, /* 3 */
    0x1c, 0x2c, 0x4c, 0xcc, 0xfc, 0x0c, 0x0c, 0x00, /* 4 */
    0xfc, 0xc0, 0xc0, 0xf8, 0x0c, 0x0c, 0xf8, 0x00, /* 5 */
    0x70, 0xc0, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00, /* 6 */
    0xfc, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x30, 0x00, /* 7 */
    0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00, /* 8 */
    0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x30, 0x00, /* 9 */
    0x00, 0xc0, 0xc0, 0x00, 0x00, 0xc0, 0xc0, 0x00, /* : */
    0x00, 0xc0, 0xc0, 0x00, 0x00, 0x40, 0xc0, 0x80, /* ; */
    0x10, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x10, 0x00, /* < */
    0x00, 0x00, 0xf0, 0x00, 0xf0, 0x00, 0x00, 0x00, /* = */
    0x80, 0xc0, 0x60, 0x30, 0x60, 0xc0, 0x80, 0x00, /* > */
    0x78, 0x8c, 0x1c, 0x38, 0x30, 0x00, 0x30, 0x00, /* ? */
    0x7c, 0xc6, 0xde, 0xd6, 0xde, 0xc0, 0x78, 0x00, /* @ */
    0x38, 0x4c, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00, /* A */
    0xf8, 0xcc, 0xcc, 0xf8, 0xcc, 0xcc, 0xf8, 0x00, /* B */
    0x78, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x78, 0x00, /* C */
    0xf0, 0xd8, 0xcc, 0xcc, 0xcc, 0xcc, 0xf8, 0x00, /* D */
    0x78, 0xc0, 0xc0, 0xf0, 0xc0, 0xc0, 0x78, 0x00, /* E */
    0x78, 0xc0, 0xc0, 0xf0, 0xc0, 0xc0, 0xc0, 0x00, /* F */
    0x78, 0xc0, 0xc0, 0xdc, 0xcc, 0xcc, 0x78, 0x00, /* G */
    0x4c, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00, /* H */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, /* I */
    0x18, 0x18, 0x18, 0x18, 0x98, 0xd8, 0x70, 0x00, /* J */
    0x44, 0xcc, 0xd8, 0xf0, 0xf0, 0xd8, 0xcc, 0x00, /* K */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xf8, 0x00, /* L */
    0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6, 0x00, /* M */
    0xcc, 0xcc, 0xec, 0xfc, 0xdc, 0xcc, 0xcc, 0x00, /* N */
    0x78, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00, /* O */
    0xf8, 0xcc, 0xcc, 0xf8, 0xc0, 0xc0, 0xc0, 0x00, /* P */
    0x38, 0x4c, 0xcc, 0xcc, 0xcc, 0xd8, 0x7c, 0x00, /* Q */
    0x78, 0xcc, 0xcc, 0xf8, 0xd8, 0xcc, 0xcc, 0x00, /* R */
    0x78, 0xc0, 0xe0, 0x70, 0x38, 0x18, 0xf0, 0x00, /* S */
    0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, /* T */
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00, /* U */
    0xcc, 0xcc, 0xcc, 0xcc, 0x48, 0x78, 0x30, 0x00, /* V */
    0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00, /* W */
    0xcc, 0xcc, 0x78, 0x30, 0x78, 0xcc, 0xcc, 0x00, /* X */
    0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x30, 0x00, /* Y */
    0xf8, 0x18, 0x38, 0x70, 0xe0, 0xc0, 0xf8, 0x00, /* Z */
    0xe0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0x00, /* [ */
    0xc0, 0xc0, 0x60, 0x60, 0x60, 0x30, 0x30, 0x00, /* \ */
    0xe0, 0x60, 0x60, 0x60, 0x60, 0x60, 0xe0, 0x00, /* ] */
    0x60, 0xf0, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, /* ^ */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, /* _ */
    0xc0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ` */
    0x00, 0x00, 0x78, 0x0c, 0xfc, 0xcc, 0x7c, 0x00, /* a */
    0xc0, 0xc0, 0xf8, 0xcc, 0xcc, 0xcc, 0x78, 0x00, /* b */
    0x00, 0x00, 0x78, 0xc0, 0xc0, 0xc0, 0x78, 0x00, /* c */
    0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0xcc, 0x78, 0x00, /* d */
    0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00, /* e */
    0x38, 0x60, 0xf8, 0x60, 0x60, 0x60, 0x60, 0x00, /* f */
    0x00, 0x70, 0xd8, 0xd8, 0x70, 0xfc, 0xcc, 0x78, /* g */
    0xc0, 0xc0, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, /* h */
    0xc0, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, /* i */
    0x60, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0xc0, /* j */
    0xc0, 0xc0, 0xc8, 0xd8, 0xf0, 0xf0, 0xd8, 0x00, /* k */
    0x40, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x00, /* l */
    0x00, 0x00, 0x7c, 0xd6, 0xd6, 0xd6, 0xc6, 0x00, /* m */
    0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, /* n */
    0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00, /* o */
    0x00, 0x00, 0x78, 0xcc, 0xcc, 0xf8, 0xc0, 0xc0, /* p */
    0x00, 0x00, 0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x0c, /* q */
    0x00, 0x00, 0xd0, 0xe0, 0xc0, 0xc0, 0xc0, 0x00, /* r */
    0x00, 0x00, 0x7c, 0xe0, 0x78, 0x1c, 0xf8, 0x00, /* s */
    0x20, 0x60, 0xf8, 0x60, 0x60, 0x60, 0x38, 0x00, /* t */
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c, 0x00, /* u */
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00, /* v */
    0x00, 0x00, 0xc6, 0xd6, 0xd6, 0xd6, 0x7c, 0x00, /* w */
    0x00, 0x00, 0xcc, 0x78, 0x30, 0x78, 0xcc, 0x00, /* x */
    0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x18, 0x70, /* y */
    0x00, 0x00, 0xf8, 0x30, 0x60, 0xc0, 0xf8, 0x00, /* z */
    0x30, 0x60, 0x60, 0xe0, 0x60, 0x60, 0x30, 0x00, /* { */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, /* | */
    0xc0, 0x60, 0x60, 0x70, 0x60, 0x60, 0xc0, 0x00, /* } */
    0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ~ */
    0x00, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0x00 /*  */
};
static const uint8_t fnt_6_width[] = {
    0x2, /*   */
    0x3, /* ! */
    0x4, /* " */
    0x8, /* # */
    0x7, /* $ */
    0x7, /* % */
    0x8, /* & */
    0x3, /* ' */
    0x4, /* ( */
    0x4, /* ) */
    0x6, /* * */
    0x6, /* + */
    0x3, /* , */
    0x5, /* - */
    0x3, /* . */
    0x5, /* / */
    0x7, /* 0 */
    0x7, /* 1 */
    0x7, /* 2 */
    0x7, /* 3 */
    0x7, /* 4 */
    0x7, /* 5 */
    0x7, /* 6 */
    0x7, /* 7 */
    0x7, /* 8 */
    0x7, /* 9 */
    0x3, /* : */
    0x3, /* ; */
    0x5, /* < */
    0x5, /* = */
    0x5, /* > */
    0x7, /* ? */
    0x8, /* @ */
    0x7, /* A */
    0x7, /* B */
    0x6, /* C */
    0x7, /* D */
    0x6, /* E */
    0x6, /* F */
    0x7, /* G */
    0x7, /* H */
    0x3, /* I */
    0x6, /* J */
    0x7, /* K */
    0x6, /* L */
    0x8, /* M */
    0x7, /* N */
    0x7, /* O */
    0x7, /* P */
    0x7, /* Q */
    0x7, /* R */
    0x6, /* S */
    0x7, /* T */
    0x7, /* U */
    0x7, /* V */
    0x8, /* W */
    0x7, /* X */
    0x7, /* Y */
    0x6, /* Z */
    0x4, /* [ */
    0x5, /* \ */
    0x4, /* ] */
    0x5, /* ^ */
    0x6, /* _ */
    0x4, /* ` */
    0x7, /* a */
    0x7, /* b */
    0x6, /* c */
    0x7, /* d */
    0x7, /* e */
    0x6, /* f */
    0x7, /* g */
    0x7, /* h */
    0x3, /* i */
    0x4, /* j */
    0x6, /* k */
    0x4, /* l */
    0x8, /* m */
    0x7, /* n */
    0x7, /* o */
    0x7, /* p */
    0x7, /* q */
    0x5, /* r */
    0x7, /* s */
    0x6, /* t */
    0x7, /* u */
    0x7, /* v */
    0x8, /* w */
    0x7, /* x */
    0x7, /* y */
    0x6, /* z */
    0x5, /* { */
    0x3, /* | */
    0x5, /* } */
    0x8, /* ~ */
    0x5 /*  */
};

static inline uint16_t drawChar(uint32_t* fb, uint16_t xp, uint16_t yp, uint16_t w, uint16_t h, const char c, const uint32_t color)
{
    if ((c < 0x20) || (c > 0x7f))
        return xp;
    int pixelpos_max = h * w;
    const uint8_t* bmp = &fnt_6_blcd[(c - 0x20) * 8];
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((bmp[y] >> (7 - x)) & 0x1) {
                int pixelpos = (yp + y) * w + xp + x;
                if (pixelpos < pixelpos_max)
                    fb[pixelpos] = color;
            }
        }
    }
    return xp += fnt_6_width[c - 0x20];
}
void drawString(Widget* v, uint16_t xp, uint16_t yp, const char* str, const uint32_t color)
{
    uint16_t w = v->surface->w;
    uint16_t h = v->surface->h;
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    const char* c = str;
    uint16_t x = xp;
    while (*c) {
        x = drawChar(bmp, x, yp, w, h, *c, color);
        c++;
    }
}

void drawStringCentered(Widget* v, uint16_t xp, uint16_t yp, const char* str, const uint32_t color)
{
    const char* c = str;
    uint16_t len = 0;
    while (*c) {
        len += fnt_6_width[*c - 0x20];
        c++;
    }
    drawString(v, xp - len / 2, yp, str, color);
}

void drawU16Centered(Widget* v, uint16_t xp, uint16_t yp, uint16_t value, const uint32_t color)
{
    uint16_t w = v->surface->w;
    uint16_t h = v->surface->h;
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    const char toh[17] = "0123456789ABCDEF";
    for (int i = 0; i < 4; i++) {
        drawChar(bmp, xp + (1 - i) * 7, yp, w, h, toh[(value >> (i * 4)) & 0xF], color);
    }
}

void drawOutline(Widget* v, const uint32_t color)
{
    int xe = v->surface->w;
    int ye = v->surface->h;
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    for (int x = 0; x < xe; x++) {
        bmp[x] = color;
        bmp[(ye - 1) * xe + x] = color;
    }
    for (int y = 0; y < ye; y++) {
        bmp[y * xe] = color;
        bmp[y * xe + xe - 1] = color;
    }
}

void drawLedFill(Widget* v, const uint32_t color)
{
    int w = v->surface->w;
    int h = v->surface->h;
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    uint16_t lye = h / 5;
    for (int y = 1; y < lye; y++) {
        for (int x = 1; x < w - 1; x++) {
            bmp[y * w + x] = v->led | (panel.widget_led_alpha << 24);
        }
    }
    for (int y = lye; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            bmp[y * w + x] = (color & 0x00FFFFFF) | (panel.widget_fill_alpha << 24);
        }
    }
}

// put pixel for circle
static inline void pp(uint16_t x, uint16_t y, uint16_t d, uint32_t* bmp, uint32_t color)
{
    uint32_t pos = d * y + x;
    bmp[pos] = color;
}

// d is size of wid (w == h)
void drawCircle(Widget* v, uint8_t y_bottom_cut, uint32_t color)
{
    uint16_t d = v->surface->w;
    uint16_t y_lim = d - y_bottom_cut;
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    uint16_t r = d / 2 - 1;
    uint16_t r1 = d / 2;
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (r + y < y_lim) {
            pp(r + x, r + y, d, bmp, color);
            pp(r1 - x, r + y, d, bmp, color);
        }
        pp(r + x, r1 - y, d, bmp, color);
        pp(r1 - x, r1 - y, d, bmp, color);
        if (r + x < y_lim) {
            pp(r + y, r + x, d, bmp, color);
            pp(r1 - y, r + x, d, bmp, color);
        }
        pp(r + y, r1 - x, d, bmp, color);
        pp(r1 - y, r1 - x, d, bmp, color);
    }
}

// line, but it uses circle-flavored pp
void drawLine(Widget* v, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t d, uint32_t color)
{
    uint32_t* bmp = (uint32_t*)v->surface->pixels;
    int16_t steep = SDL_abs(y1 - y0) > SDL_abs(x1 - x0);
    if (steep) {
        int16_t t = x0;
        x0 = y0, y0 = t;
        t = x1;
        x1 = y1, y1 = t;
    }
    if (x0 > x1) {
        int16_t t = x0;
        x0 = x1, x1 = t;
        t = y0;
        y0 = y1, y1 = t;
    }
    int16_t dx, dy;
    dx = x1 - x0;
    dy = SDL_abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (; x0 <= x1; x0++) {
        if (steep) {
            pp(y0, x0, d, bmp, color);
        } else {
            pp(x0, y0, d, bmp, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

#define PI_F 3.14159265358979323846264338327950288f

#endif // __WID_GRAPHICS_H