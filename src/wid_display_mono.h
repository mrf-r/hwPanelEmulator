#ifndef __WID_DISPLAY_MONO_H
#define __WID_DISPLAY_MONO_H

#include "mgl.h"
#include "wid_graphics.h"
#include "widget.h"

// display
#define DISPM_COLOR_BACK 0x000000FF // blue inverted
#define DISPM_COLOR_PIXEL 0xFFFFFF
// #define DISPM_COLOR_BACK 0x0040C000 // green standart
// #define DISPM_COLOR_PIXEL 0x0
#define DISPM_ALPHA 0xA0
#define DISPM_GAP 2
extern const MglFont _5x7mod;

typedef struct {
    Widget v;
    uint32_t color_pix;
    uint16_t waxs;
    uint16_t ways;
    uint16_t waxe;
    uint16_t waye;
    uint16_t x;
    uint16_t y;
    uint8_t* framebuffer;
    MglDisplay* disp;
} WidgetDisplayMono;

#define WID_DISPLAY_MONO_DEFINE(inst_name, w, h)                                                                              \
    static WidgetDisplayMono inst_name;                                                                                       \
    static uint8_t inst_name##_framebuffer[(w * h + 7) / 8];                                                                  \
    static void inst_name##_SetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size) \
    {                                                                                                                         \
        inst_name.waxs = wax, inst_name.ways = way, inst_name.waxe = wax + wax_size, inst_name.waye = way + way_size;         \
        MGL_ASSERT(inst_name.waxs < w);                                                                                       \
        MGL_ASSERT(inst_name.waxe <= w);                                                                                      \
        MGL_ASSERT(inst_name.ways < h);                                                                                       \
        MGL_ASSERT(inst_name.waye <= h);                                                                                      \
        inst_name.x = inst_name.waxs, inst_name.y = inst_name.ways;                                                           \
    }                                                                                                                         \
    static void inst_name##_PixelOut(MglColor c)                                                                              \
    {                                                                                                                         \
        if (c.wrd == COLOR_OFF.wrd)                                                                                           \
            inst_name##_framebuffer[inst_name.y / 8 * w + inst_name.x] &= ~(1 << (inst_name.y & 0x7));                        \
        else if (c.wrd == COLOR_ON.wrd)                                                                                       \
            inst_name##_framebuffer[inst_name.y / 8 * w + inst_name.x] |= 1 << (inst_name.y & 0x7);                           \
        else if (c.wrd == COLOR_INVERT.wrd)                                                                                   \
            inst_name##_framebuffer[inst_name.y / 8 * w + inst_name.x] ^= 1 << (inst_name.y & 0x7);                           \
        inst_name.x++;                                                                                                        \
        if (inst_name.x >= inst_name.waxe) {                                                                                  \
            inst_name.x = inst_name.waxs;                                                                                     \
            inst_name.y++;                                                                                                    \
            if (inst_name.y >= inst_name.waye) {                                                                              \
                inst_name.y = inst_name.ways;                                                                                 \
            }                                                                                                                 \
        }                                                                                                                     \
        inst_name.v.need_redraw = 1;                                                                                          \
    }                                                                                                                         \
    static void inst_name##_Update()                                                                                          \
    {                                                                                                                         \
        inst_name.v.need_redraw = 1;                                                                                          \
    }                                                                                                                         \
    static MglDispContext inst_name##_mglcontext;                                                                             \
    static MglDisplay inst_name##_mgldisp = {                                                                                 \
        .context = &inst_name##_mglcontext,                                                                                   \
        .size_x = w,                                                                                                          \
        .size_y = h,                                                                                                          \
        .setZone = inst_name##_SetZone,                                                                                       \
        .pixelOut = inst_name##_PixelOut,                                                                                     \
        .update = inst_name##_Update                                                                                          \
    };

static void wDispMonoRedraw(void* wid)
{
    WidgetDisplayMono* v = (WidgetDisplayMono*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        drawOutline(&v->v, panel.widget_color_released);

        MglColor led = { .wrd = (v->v.led & 0x00FFFFFF) | (DISPM_ALPHA << 24) };
        MglColor pix = { .wrd = (v->color_pix & 0x00FFFFFF) | (DISPM_ALPHA << 24) };
        pix = mgAlphablend(0x60, pix, led);
        pix.alpha = DISPM_ALPHA;
        uint32_t* framebuffer = (uint32_t*)v->v.surface->pixels;
        // draw gap pixels
        uint8_t gap = DISPM_GAP + 1;
        for (int i = 0; i < DISPM_GAP; i++) {
            for (int x = 1; x < v->v.surface->w - 1; x++) {
                framebuffer[v->v.surface->w * (1 + i) + x] = led.wrd;
                framebuffer[v->v.surface->w * (v->v.surface->h - i - 2) + x] = led.wrd;
            }
            for (int y = 1; y < v->v.surface->h - 1; y++) {
                framebuffer[v->v.surface->w * y + i + 1] = led.wrd;
                framebuffer[v->v.surface->w * y + v->v.surface->w - i - 2] = led.wrd;
            }
        }
        // draw actual pixels
        for (int y = 0; y < v->disp->size_y; y++) {
            for (int x = 0; x < v->disp->size_x; x++) {
                uint32_t fbpos_mono = y / 8 * v->disp->size_x + x;
                uint32_t fbshift_mono = y & 0x7;
                uint32_t p = (v->framebuffer[fbpos_mono] >> fbshift_mono) & 0x1;
                p = (p == 1) ? pix.wrd : led.wrd;
                framebuffer[(y + gap) * v->v.surface->w + (x + gap)] = p;
            }
        }
    }
}

static WidgetApi wDispMonoApi = {
    .redraw = wDispMonoRedraw,
    .process = 0,
    .keyboard = 0,
    .mouseMove = 0,
    .mouseClick = 0,
    .mouseWheel = 0
};

__attribute__((unused)) static void wDisplayMonoInit(
    WidgetDisplayMono* v,
    MglDisplay* disp,
    uint8_t* framebuffer,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend)
{
    SDL_memset(disp->context, 0, sizeof(MglDispContext));
    SDL_memset(v, 0, sizeof(WidgetDisplayMono));
    uint8_t gap = DISPM_GAP + 1;
    widgetInit(&v->v, (void*)v, &wDispMonoApi, x, y, disp->size_x + 2 * gap, disp->size_y + 2 * gap, scale, rend);
    v->disp = disp;
    v->framebuffer = framebuffer;
    v->color_pix = DISPM_COLOR_PIXEL;
    widgetLed(&v->v, DISPM_COLOR_BACK);
    mgsDisplay(disp);
    mgsFont(&_5x7mod);
    mgsBackColor(COLOR_OFF);
    mgsAlign(MGL_ALIGN_LEFT);
}

#endif // __WID_DISPLAY_MONO_H