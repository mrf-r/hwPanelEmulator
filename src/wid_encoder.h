#ifndef __WID_ENCODER_H
#define __WID_ENCODER_H

#include "widget.h"

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

void wEncInit(
    WidgetEnc* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode_inc,
    SDL_KeyCode keycode_dec,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);

#endif // __WID_ENCODER_H