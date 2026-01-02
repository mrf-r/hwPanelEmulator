#ifndef __WID_BUTTON_H
#define __WID_BUTTON_H

#include "widget.h"

typedef struct {
    Widget v;
    const char* name;
    uint8_t midictrl;
    const char* kbd;
    SDL_KeyCode keycode;
    uint8_t pointed;
} WidgetButton;

void wButtonInit(
    WidgetButton* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode,
    uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);

#endif // __WID_BUTTON_H