#ifndef __WID_BUTTON_H
#define __WID_BUTTON_H

#include "wid_graphics.h"
#include "widget.h"

// __attribute__((weak)) void wButtonMidiSend(uint8_t midictrl, uint8_t value)
// {
//     (void)midictrl;
//     (void)value;
// }
void wButtonMidiSend(uint8_t midictrl, uint8_t value);

#define BUT_SRC_POINTED 0x01
#define BUT_SRC_MOUSE 0x02
#define BUT_SRC_KEYBD 0x04
// #define BUT_SRC_TOUCH 0x08
// #define BUT_SRC_EXT 0x10
#define BUT_SRC_INT 0x80

typedef struct {
    Widget v;
    const char* name;
    uint8_t midictrl;
    const char* kbd;
    SDL_KeyCode keycode;
    uint8_t pointed;
} WidgetButton;

static void wButtonRedraw(void* wid)
{
    WidgetButton* v = (WidgetButton*)wid;
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
        uint16_t d = v->v.surface->w;
        drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
        drawString(&v->v, d / 6, d * 2 / 3, v->kbd, widget_color_helptext);
    }
}
static void wButtonProcess(void* wid, uint32_t ms)
{
    WidgetButton* v = (WidgetButton*)wid;
    (void)ms;
    const uint8_t point_mask = (uint8_t) ~(BUT_SRC_INT | BUT_SRC_POINTED);
    if ((v->pointed & BUT_SRC_INT) && (0 == (v->pointed & point_mask))) {
        v->pointed &= ~BUT_SRC_INT;
        wButtonMidiSend(v->midictrl, 0);
    } else if ((0 == (v->pointed & BUT_SRC_INT)) && (v->pointed & point_mask)) {
        v->pointed |= BUT_SRC_INT;
        wButtonMidiSend(v->midictrl, 0x7F);
    }
}
static void wButtonKeyboard(void* wid, SDL_Event* e)
{
    WidgetButton* v = (WidgetButton*)wid;
    uint8_t pointed = v->pointed;
    if (e->key.keysym.sym == v->keycode) {
        if (e->type == SDL_KEYDOWN) {
            pointed |= BUT_SRC_KEYBD;
        } else if (e->type == SDL_KEYUP) {
            pointed &= ~BUT_SRC_KEYBD;
        }
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wButtonMouseMove(void* wid, SDL_Point* pos, uint8_t click)
{
    WidgetButton* v = (WidgetButton*)wid;
    uint8_t pointed = v->pointed;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        pointed |= BUT_SRC_POINTED;
        if (click)
            pointed |= BUT_SRC_MOUSE;
        else
            pointed &= ~BUT_SRC_MOUSE;
    } else {
        pointed &= ~(BUT_SRC_MOUSE | BUT_SRC_POINTED);
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wButtonMouseClick(void* wid, SDL_Point* pos, Drag* d)
{
    WidgetButton* v = (WidgetButton*)wid;
    (void)d;
    uint8_t pointed = v->pointed;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        pointed |= BUT_SRC_MOUSE | BUT_SRC_POINTED;
    } else {
        pointed &= ~(BUT_SRC_MOUSE | BUT_SRC_POINTED);
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}

static WidgetApi wButtonApi = {
    .redraw = wButtonRedraw,
    .process = wButtonProcess,
    .keyboard = wButtonKeyboard,
    .mouseMove = wButtonMouseMove,
    .mouseClick = wButtonMouseClick,
    .mouseWheel = 0
};

static void wButtonInit(
    WidgetButton* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode,
    uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wButtonApi, x, y, widget_unit_size, widget_unit_size, widget_scale, rend);
    v->name = name;
    v->kbd = kbd;
    v->keycode = keycode;
    v->midictrl = midictrl;
    v->pointed = 0;
}

#endif // __WID_BUTTON_H