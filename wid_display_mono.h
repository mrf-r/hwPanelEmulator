#ifndef __WID_DISPLAY_MONO_H
#define __WID_DISPLAY_MONO_H

#include "widget.h"
#include "mgl.h"

// #define DISPLAY_SCREENSHOT_DISABLE_ALPHA

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

// please, use this macro to create a display object
// this will creates minimalgraphics api
// all you need in your app is mgl library with a pointer to MglDisplay <name>_mgldisp obejct
// colors are COLOR_OFF, COLOR_ON, or COLOR_INVERT
// w, h - width and height in pixels (ex. 128, 64 - for popular oled panel)
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

void wDisplayMonoInit(
    WidgetDisplayMono* v,
    MglDisplay* disp,
    uint8_t* framebuffer,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend);

#endif // __WID_DISPLAY_MONO_H