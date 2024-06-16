#ifndef __WIDGET_H
#define __WIDGET_H

#include <SDL.h>

typedef struct
{
    SDL_Point point;
    SDL_bool is_pressed;
    SDL_FingerID finger;
    void* instance;
    void (*drag)(void* instance, int32_t delta);
} WidgetTouchData;

typedef struct {
    // redraw called each frame
    void (*redraw)(void* wid);
    // process potentially not tied to frame rate, can be called more often or less often
    void (*process)(void* wid, uint32_t clock);
    void (*keyboard)(void* wid, SDL_Event* e);
    void (*touchMove)(void* wid, WidgetTouchData* d, unsigned touch_elements);
    void (*touchClick)(void* wid, WidgetTouchData* d);
    void (*mouseWheel)(void* wid, SDL_Point* pos, int32_t delta);
    void (*terminate)(void* wid);
} WidgetApi;

typedef struct Widget_ {
    SDL_Rect rect;
    SDL_Surface* surface;
    SDL_Texture* texture;
    struct Widget_* next;
    uint32_t led;
    uint8_t need_redraw;
    WidgetApi* api;
    void* parent;
} Widget;

typedef struct {
    Widget* list_start;
    Widget* list_end;
    uint8_t widget_unit_size;
    uint8_t widget_scale;
    uint8_t widget_led_alpha;
    uint8_t widget_fill_alpha;
    uint32_t widget_color_panel;
    uint32_t widget_color_released; // normal outline
    uint32_t widget_color_pointed;
    uint32_t widget_color_pressed; //
    uint32_t widget_color_helptext; // values or keyboard keys info
} Panel;

extern Panel panel;
extern uint32_t lcg;

static inline uint32_t widgetRandom()
{
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

static inline void widgetLed(Widget* v, uint32_t color)
{
    v->led = (color & 0x00FFFFFF) | (panel.widget_led_alpha << 24);
    v->need_redraw = 1;
}

void widgetInit(
    Widget* v,
    void* parent,
    WidgetApi* api,
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    uint8_t scale,
    SDL_Renderer* rend);

#endif // __WIDGET_H