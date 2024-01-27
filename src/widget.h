#ifndef __VIDGET_H
#define __VIDGET_H

#include "SDL.h"

typedef struct
{
    void* instance;
    void (*drag)(void* instance, int32_t delta);
} Drag;

typedef struct {
    // redraw called each frame
    void (*redraw)(void* wid);
    // process potentially not tied to frame rate, can be called more often or less often
    void (*process)(void* wid, uint32_t ms);
    void (*keyboard)(void* wid, SDL_Event* e);
    void (*mouseMove)(void* wid, SDL_Point* pos, uint8_t click);
    void (*mouseClick)(void* wid, SDL_Point* pos, Drag* d);
    void (*mouseWheel)(void* wid, SDL_Point* pos, int32_t delta);
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

extern Widget* list_start;
extern Widget* list_end;
extern uint8_t widget_unit_size;

extern uint8_t widget_scale;
extern uint8_t widget_led_alpha;
extern uint8_t widget_fill_alpha;
extern uint32_t widget_color_panel;
extern uint32_t widget_color_released; // normal outline
extern uint32_t widget_color_pointed;
extern uint32_t widget_color_pressed; //
extern uint32_t widget_color_helptext; // values or keyboard keys info

static inline uint32_t widgetRandom()
{
    static uint32_t lcg = 0;
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

static inline void widgetLed(Widget* v, uint32_t color)
{
    v->led = (color & 0x00FFFFFF) | (widget_led_alpha << 24);
    v->need_redraw = 1;
}

static inline void widgetInit(
    Widget* v,
    void* parent,
    WidgetApi* api,
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    uint8_t scale,
    SDL_Renderer* rend)
{
    v->parent = parent;
    v->api = api;
    v->rect.x = x;
    v->rect.y = y;
    v->rect.w = w * scale;
    v->rect.h = h * scale;
    v->surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    v->texture = SDL_CreateTextureFromSurface(rend, v->surface);
    v->next = 0;
    if (0 == list_start) {
        list_start = v;
        list_end = v;
    } else {
        list_end->next = v;
        list_end = v;
    }
    widgetLed(v, 0);
}

static inline void widgetFreeAll()
{
    Widget* v = list_start;
    while (0 != v) {
        SDL_DestroyTexture(v->texture);
        SDL_FreeSurface(v->surface);
        v = v->next;
    }
}

static inline void widgetRenderAll(SDL_Renderer* rend)
{
    Widget* v = list_start;
    while (0 != v) {
        SDL_UpdateTexture(v->texture, 0, v->surface->pixels, v->surface->pitch);
        SDL_RenderCopy(rend, v->texture, 0, &v->rect);
        v = v->next;
    }
}

static inline void widgetRedrawAll()
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->redraw)
            v->api->redraw(v->parent);
        v = v->next;
    }
}
static inline void widgetProcessAll(uint32_t ms)
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->process)
            v->api->process(v->parent, ms);
        v = v->next;
    }
}
static inline void widgetKeyboardAll(SDL_Event* e)
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->keyboard)
            v->api->keyboard(v->parent, e);
        v = v->next;
    }
}
static inline void widgetMouseMoveAll(SDL_Point* pos, uint8_t click)
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->mouseMove)
            v->api->mouseMove(v->parent, pos, click);
        v = v->next;
    }
}
static inline void widgetMouseClickAll(SDL_Point* pos, Drag* d)
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->mouseClick)
            v->api->mouseClick(v->parent, pos, d);
        v = v->next;
    }
}
static inline void widgetMouseWheelAll(SDL_Point* pos, int32_t delta)
{
    Widget* v = list_start;
    while (0 != v) {
        if (v->api->mouseWheel)
            v->api->mouseWheel(v->parent, pos, delta);
        v = v->next;
    }
}

#endif // __VIDGET_H