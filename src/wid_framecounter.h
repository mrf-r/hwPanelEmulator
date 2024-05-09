#ifndef __WID_FRAMECOUNTER_H
#define __WID_FRAMECOUNTER_H

#include "widget.h"

typedef struct {
    Widget v;
    uint32_t prev_frame;
    uint16_t counter_proc;
    uint16_t counter_redraw;
    uint16_t frames_proc;
    uint16_t frames_redraw;
} WidgetFrameCounter;

void wFrameCounterInit(
    WidgetFrameCounter* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);

#endif // __WID_FRAMECOUNTER_H