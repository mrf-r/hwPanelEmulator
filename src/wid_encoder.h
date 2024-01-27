#ifndef __WID_ENCODER_H
#define __WID_ENCODER_H

#include "wid_graphics.h"
#include "widget.h"

__attribute__((weak)) void wEncMidiSend(uint8_t midictrl, uint8_t value)
{
    (void)midictrl;
    (void)value;
}
// void wEncMidiSend(uint8_t midictrl, uint8_t value);

#define ENCODER_NOTCHES 7
#define ENCODER_SHIFT 16

typedef struct {
    Widget v;
    const char* name;
    uint8_t midictrl;
    const char* kbd;
    SDL_KeyCode keycode_inc;
    SDL_KeyCode keycode_dec;

    uint8_t pointed;
    uint16_t value_drag;
    uint16_t value_send;
} WidgetEnc;

static void wEncRedraw(void* wid)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        uint32_t color = widget_color_released;
        if (v->pointed) {
            if (v->pointed > 1)
                color = widget_color_pressed;
            else
                color = widget_color_pointed;
        }
        drawOutline(&v->v, color);
        drawLedFill(&v->v, color);
        drawCircle(&v->v, 0, color);
        uint16_t d = v->v.surface->w;
        for (int i = 0; i < ENCODER_NOTCHES; i++) {
            float value = -(uint16_t)(v->value_drag << 4) + (65536 / ENCODER_NOTCHES) * i;
            float angle = (float)value * (1.f / (65536.f)) * (PI_F * 2.f);
            float x = SDL_sinf(angle);
            float y = SDL_cosf(angle);
            int32_t xs = x * (float)(d / 2 - 2) + d / 2;
            int32_t xe = x * (float)(d / 3) + d / 2;
            int32_t ys = y * (float)(d / 2 - 2) + d / 2;
            int32_t ye = y * (float)(d / 3) + d / 2;
            drawLine(&v->v, xs, ys, xe, ye, d, color);
        }
        drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
    }
}
static void wEncProcess(void* wid, uint32_t ms)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    (void)ms;
    int16_t d = v->value_drag - v->value_send;
    int16_t ds = d / ENCODER_SHIFT;
    if (0 != ds) {
        if (ds > 63)
            ds = 63;
        else if (ds < -64)
            ds = -64;
        wEncMidiSend(v->midictrl, ds);
        v->value_send += ds * ENCODER_SHIFT;
        v->v.need_redraw = 1;
    }
}
static void wEncDrag(void* instance, int32_t delta)
{
    WidgetEnc* v = (WidgetEnc*)instance;
    v->value_drag += delta;
}
static void wEncKeyboard(void* wid, SDL_Event* e)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == v->keycode_inc) {
            v->value_drag += ENCODER_SHIFT;
            v->v.need_redraw = 1;
        } else if (e->key.keysym.sym == v->keycode_dec) {
            v->value_drag -= ENCODER_SHIFT;
            v->v.need_redraw = 1;
        }
    }
}
static void wEncMouseMove(void* wid, SDL_Point* pos, uint8_t click)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    (void)click;
    uint8_t pointed = v->pointed;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        pointed = 1;
    } else {
        pointed = 0;
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wEncMouseClick(void* wid, SDL_Point* pos, Drag* d)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        v->pointed = 2;
        d->instance = (void*)v;
        d->drag = wEncDrag;
    }
}
static void wEncMouseWheel(void* wid, SDL_Point* pos, int32_t delta)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        wEncDrag(wid, delta);
    }
}
static WidgetApi wEncApi = {
    .redraw = wEncRedraw,
    .process = wEncProcess,
    .keyboard = wEncKeyboard,
    .mouseMove = wEncMouseMove,
    .mouseClick = wEncMouseClick,
    .mouseWheel = wEncMouseWheel
};

static void wEncInit(
    WidgetEnc* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode_inc,
    SDL_KeyCode keycode_dec,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wEncApi, x, y, widget_unit_size, widget_unit_size, widget_scale, rend);
    v->name = name;
    v->midictrl = midictrl;
    v->kbd = kbd;
    v->keycode_inc = keycode_inc;
    v->keycode_dec = keycode_dec;
    v->pointed = 0;
    v->value_drag = 0;
    v->value_send = 0;
}

#endif // __WID_ENCODER_H