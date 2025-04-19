#ifndef __WID_DISPLAY_MULTI_H
#define __WID_DISPLAY_MULTI_H

#include "widget.h"
#include "mgl.h"

// display
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

// please, use this macro to create a display object
// this will creates minimalgraphics api
// all you need in your app is mgl library with a pointer to MglDisplay <name>_mgldisp obejct
// internal colors are RGBA8888, but this is an emulation of the popular RGB565 with added dithering
// w, h - width and height in pixels (ex. 320, 240 - for popular tft panel)
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
        int32_t red = c.red + ((inst_name.lcg >> 20) & 0x7);                                                                  \
        c.red = (red < 255 ? red : 255) & 0xF8;                                                                               \
        int32_t green = c.green + ((inst_name.lcg >> 23) & 0x7);                                                              \
        c.green = (green < 255 ? green : 255) & 0xFC;                                                                         \
        int32_t blue = c.blue + ((inst_name.lcg >> 17) & 0x7);                                                                \
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

void wDisplayMultiInit(
    WidgetDisplayMulti* v,
    MglDisplay* disp,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend);

#endif // __WID_DISPLAY_MULTI_H