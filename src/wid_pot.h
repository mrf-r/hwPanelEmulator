#ifndef __WID_POT_H
#define __WID_POT_H

#include "widget.h"
#include "tools/potproc.h" // PotProcData

typedef struct {
    Widget v;
    const char* name;
    uint8_t midictrl;
    uint8_t pointed;
    uint16_t analog_src14b;
    int32_t filter;
    uint32_t prev_ms;
    PotProcData potdata;
    uint16_t prev_lock;
} WidgetPot;

void wPotInit(
    WidgetPot* v,
    const char* name,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);

#endif // __WID_POT_H