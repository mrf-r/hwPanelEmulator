#ifndef __WID_PIANO_H
#define __WID_PIANO_H

#include "widget.h"

#define WID_PIANO_NUM_KEYS 17

typedef struct {
    uint8_t channel;
    uint8_t note;
}WidPianoKeyState;

typedef struct {
    Widget v;
    const char* name;
    uint32_t update_bmp;
    uint32_t pointed_bmp;
    uint32_t press_mouse_bmp;
    uint32_t press_keyb_bmp;
    uint32_t press_ext_bmp;
    uint32_t press_int_bmp;
    WidPianoKeyState keys[WID_PIANO_NUM_KEYS];
    uint8_t velocity;
    uint8_t octave;
    uint8_t channel;
    uint8_t scale;
} WidgetPiano;

void wPianoInit(
    WidgetPiano* v,
    const char* name,
    uint8_t scale,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend);

#endif // __WID_PIANO_H