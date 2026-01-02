#include "wid_framecounter.h"
#include "wid_graphics.h"
#include "mbwmidi.h"

#define FRAMECOUNTER_COLOR 0xFF000000
#define FRAMECOUNTER_PERIOD_CLK_CYCLES MIDI_CLOCK_RATE

static uint32_t dec2bcd(uint16_t dec)
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

static void wFrameCounterProcess(void* wid, uint32_t clock)
{
    WidgetFrameCounter* v = (WidgetFrameCounter*)wid;
    uint32_t delta = clock - v->prev_frame;
    // TODO: overflow???
    if (delta > FRAMECOUNTER_PERIOD_CLK_CYCLES) {
        v->prev_frame += FRAMECOUNTER_PERIOD_CLK_CYCLES;
        v->frames_proc = v->counter_proc;
        v->counter_proc = 0;
        v->frames_redraw = v->counter_redraw;
        v->counter_redraw = 0;
        v->v.need_redraw = 1;
    } else {
        if (v->counter_proc < 9999)
            v->counter_proc++;
    }
    // TODO:::
    // uint64_t perf = SDL_GetPerformanceFrequency();
    // perf = SDL_GetPerformanceCounter();
}

static void wFrameCounterRedraw(void* wid)
{
    WidgetFrameCounter* v = (WidgetFrameCounter*)wid;
    if (v->counter_redraw < 9999)
        v->counter_redraw++;

    // if (v->v.need_redraw) {
    // v->v.need_redraw = 0;
    SDL_FillRect(v->v.surface, 0, 0);
    drawOutline(&v->v, panel.widget_color_released);
    uint32_t bcd = dec2bcd(v->frames_proc);
    drawU16Centered(&v->v, 20, 2, bcd, panel.widget_color_helptext);
    bcd = dec2bcd(v->frames_redraw);
    drawU16Centered(&v->v, 20, 10, bcd, panel.widget_color_helptext);
    // }
}

static WidgetApi wFrameCounterApi = {
    .redraw = wFrameCounterRedraw,
    .process = wFrameCounterProcess,
    .keyboard = 0,
    .touchMove = 0,
    .touchClick = 0, // wEncMouseClick, TODO: reset max ?
    .mouseWheel = 0,
    .terminate = 0
};

void wFrameCounterInit(
    WidgetFrameCounter* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wFrameCounterApi, x, y, 40, 19, 1, rend);
    v->prev_frame = SDL_GetTicks();
    v->counter_proc = v->counter_redraw = 0;
}
