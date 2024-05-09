#ifndef __WID_POT_H
#define __WID_POT_H

#include "widget.h"

typedef enum {
    POT_STATE_NORMAL = 0,
    POT_STATE_LOCK_INSIDE,
    POT_STATE_LOCK_HIGH,
    POT_STATE_LOCK_LOW,
} PotStateEn;

typedef struct {
    Widget v;
    const char* name;
    uint8_t midictrl;
    uint8_t pointed;
    PotStateEn state;
    uint16_t lock_value;
    uint16_t adc_source;
    int32_t filter;
    uint16_t output;
    uint32_t prev_ms;
} WidgetPot;

void wPotInit(
    WidgetPot* v,
    const char* name,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);
    
void wPotLock(WidgetPot* v, uint8_t midictrl, uint8_t is_new_val, uint16_t value);

#endif // __WID_POT_H