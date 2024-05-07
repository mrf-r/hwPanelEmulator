#ifndef __WID_FRAMECOUNTER_H
#define __WID_FRAMECOUNTER_H

#include "wid_graphics.h"
#include "widget.h"

#define FRAMECOUNTER_COLOR 0xFF000000
#define FRAMECOUNTER_PERIOD_MS 1000

typedef struct {
    Widget v;
    uint32_t prev_frame;
    uint16_t counter_proc;
    uint16_t counter_redraw;
    uint16_t frames_proc;
    uint16_t frames_redraw;
} WidgetFrameCounter;

uint32_t dec2bcd(uint16_t dec)
{
    uint32_t result = 0;
    int shift = 0;

    while (dec) {
        result += (dec % 10) << shift;
        dec = dec / 10;
        shift += 4;
    }
    return result;
}

static void wFrameCounterProcess(void* wid, uint32_t ms)
{
    WidgetFrameCounter* v = (WidgetFrameCounter*)wid;
    uint32_t delta = ms - v->prev_frame;
    if (delta > FRAMECOUNTER_PERIOD_MS) {
        v->prev_frame += FRAMECOUNTER_PERIOD_MS;
        v->frames_proc = v->counter_proc;
        v->counter_proc = 0;
        v->frames_redraw = v->counter_redraw;
        v->counter_redraw = 0;
        v->v.need_redraw = 1;
    } else {
        if (v->counter_proc < 9999)
            v->counter_proc++;
    }
}

static void wFrameCounterRedraw(void* wid)
{
    WidgetFrameCounter* v = (WidgetFrameCounter*)wid;
    if (v->counter_redraw < 9999)
        v->counter_redraw++;

    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        SDL_FillRect(v->v.surface, 0, 0);
        uint32_t bcd = dec2bcd(v->frames_proc);
        drawU16Centered(&v->v, 20, 0, bcd, panel.widget_color_helptext);
        bcd = dec2bcd(v->frames_redraw);
        drawU16Centered(&v->v, 20, 8, bcd, panel.widget_color_helptext);
    }
}

static WidgetApi wFrameCounterApi = {
    .redraw = wFrameCounterRedraw,
    .process = wFrameCounterProcess,
    .keyboard = 0,
    .mouseMove = 0,
    .mouseClick = 0, // wEncMouseClick, TODO: reset max ?
    .mouseWheel = 0
};

__attribute__((unused)) static void wFrameCounterInit(
    WidgetFrameCounter* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wFrameCounterApi, x, y, 40, 16, 1, rend);
    v->prev_frame = SDL_GetTicks();
    v->counter_proc = v->counter_redraw = 0;
}

#endif // __WID_FRAMECOUNTER_H