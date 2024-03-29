#ifndef __WID_DISPLAY_MULTI_H
#define __WID_DISPLAY_MULTI_H

#include "mgl.h"
#include "wid_graphics.h"
#include "widget.h"

// display
#define DISP_COLOR_BACK 0x000000FF // blue inverted
#define DISP_COLOR_PIXEL 0xFFFFFF
// #define DISP_COLOR_BACK 0x0040C000 // green standart
// #define DISP_COLOR_PIXEL 0x0
#define DISP_ALPHA 0xA0
#define DISP_GAP 2
extern const MglFont _5x7mod;

typedef struct {
    Widget v;
    uint16_t waxs;
    uint16_t ways;
    uint16_t waxe;
    uint16_t waye;
    uint16_t x;
    uint16_t y;
    MglDisplay* disp;
    uint32_t lcg;
} WidgetDisplayMulti;

#define WID_DISPLAY_MULTI_DEFINE(inst_name, w, h)                                                                             \
    static WidgetDisplayMulti inst_name;                                                                                      \
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
        inst_name.lcg = inst_name.lcg * 1103515245 + 12345;                                                                   \
        int32_t red = c.red + ((inst_name.lcg >> 22) & 0x7);                                                                  \
        c.red = (red < 255 ? red : 255) & 0xF8;                                                                               \
        int32_t green = c.green + ((inst_name.lcg >> 23) & 0x7);                                                              \
        c.green = (green < 255 ? green : 255) & 0xFC;                                                                         \
        int32_t blue = c.blue + ((inst_name.lcg >> 21) & 0x7);                                                                \
        c.blue = (blue < 255 ? blue : 255) & 0xF8;                                                                            \
        uint32_t* framebuffer = (uint32_t*)inst_name.v.surface->pixels;                                                       \
        framebuffer[(inst_name.y + 1) * (w + 2) + inst_name.x + 1] = c.wrd | 0xFF000000;                                      \
        inst_name.x++;                                                                                                        \
        if (inst_name.x >= inst_name.waxe) {                                                                                  \
            inst_name.x = inst_name.waxs;                                                                                     \
            inst_name.y++;                                                                                                    \
            if (inst_name.y >= inst_name.waye) {                                                                              \
                inst_name.y = inst_name.ways;                                                                                 \
            }                                                                                                                 \
        }                                                                                                                     \
    }                                                                                                                         \
    static void inst_name##_Update() { }                                                                                      \
    static MglDispContext inst_name##_mglcontext;                                                                             \
    static MglDisplay inst_name##_mgldisp = {                                                                                 \
        .context = &inst_name##_mglcontext,                                                                                   \
        .size_x = w,                                                                                                          \
        .size_y = h,                                                                                                          \
        .setZone = inst_name##_SetZone,                                                                                       \
        .pixelOut = inst_name##_PixelOut,                                                                                     \
        .update = inst_name##_Update                                                                                          \
    };

static void wDispMultiRedraw(void* wid)
{
    WidgetDisplayMulti* v = (WidgetDisplayMulti*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        drawOutline(&v->v, widget_color_released);
    }
}

static WidgetApi wDispMultiApi = {
    .redraw = wDispMultiRedraw,
    .process = 0,
    .keyboard = 0,
    .mouseMove = 0,
    .mouseClick = 0,
    .mouseWheel = 0
};

static void wDisplayMultiInit(
    WidgetDisplayMulti* v,
    MglDisplay* disp,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend)
{
    SDL_memset(disp->context, 0, sizeof(MglDispContext));
    SDL_memset(v, 0, sizeof(WidgetDisplayMulti));
    widgetInit(&v->v, (void*)v, &wDispMultiApi, x, y, disp->size_x + 2, disp->size_y + 2, scale, rend);
    v->disp = disp;
    drawOutline(&v->v, widget_color_released);
    mgsDisplay(disp);
    mgsFont(&_5x7mod);
    MglColor back = { .wrd = 0xFF000000 };
    mgsBackColor(back);
    mgsAlign(MGL_ALIGN_LEFT);
}

#endif // __WID_DISPLAY_MULTI_H