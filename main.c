#include <SDL.h>
#include "widget.h"
#include "panel_conf.h"

#ifndef PANEL_FPS
#define PANEL_FPS 60
#endif
#ifndef PANEL_COLOR
#define PANEL_COLOR 0xFF7D7F6C
#endif
#ifndef PANEL_WIDGET_COLOR_RELEASED
#define PANEL_WIDGET_COLOR_RELEASED 0xFF000000
#endif
#ifndef PANEL_WIDGET_COLOR_POINTED
#define PANEL_WIDGET_COLOR_POINTED 0xFF38400A
#endif
#ifndef PANEL_WIDGET_COLOR_PRESSED
#define PANEL_WIDGET_COLOR_PRESSED 0xFF900018
#endif
#ifndef PANEL_WIDGET_COLOR_HELPTEXT
#define PANEL_WIDGET_COLOR_HELPTEXT 0xFF483010
#endif
#ifndef PANEL_WIDGET_FILL_ALPHA
#define PANEL_WIDGET_FILL_ALPHA 0x20
#endif
#ifndef PANEL_WIDGET_LED_ALPHA
#define PANEL_WIDGET_LED_ALPHA 0x80
#endif
#ifndef PANEL_SCALE
#define PANEL_SCALE 2
#endif
#ifndef PANEL_UNIT_SIZE
#define PANEL_UNIT_SIZE 46
#endif


// --- to be implemented by user
void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t clock);
// ---

uint32_t lcg;
Panel panel;

#define DEBUG_PRINTF(...)
// #define DEBUG_PRINTF printf

#ifndef MULTITOUCH_MAX_FINGERS
#define MULTITOUCH_MAX_FINGERS 4
#endif

#define TOUCHPOINT_MOUSE (MULTITOUCH_MAX_FINGERS)
#define TOUCHPOINT_FINGER(x) (x)
#define TOUCHPOINTS_TOTAL (MULTITOUCH_MAX_FINGERS + 1)

static void panelInit()
{
    panel.list_start = 0;
    panel.list_end = 0;
    panel.widget_unit_size = PANEL_UNIT_SIZE;
    panel.widget_scale = PANEL_SCALE;
    panel.widget_led_alpha = PANEL_WIDGET_LED_ALPHA;
    panel.widget_fill_alpha = PANEL_WIDGET_FILL_ALPHA;
    panel.widget_color_panel = PANEL_COLOR;
    panel.widget_color_released = PANEL_WIDGET_COLOR_RELEASED;
    panel.widget_color_pointed = PANEL_WIDGET_COLOR_POINTED;
    panel.widget_color_pressed = PANEL_WIDGET_COLOR_PRESSED;
    panel.widget_color_helptext = PANEL_WIDGET_COLOR_HELPTEXT;
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
    if (0 == panel.list_start) {
        panel.list_start = v;
        panel.list_end = v;
    } else {
        panel.list_end->next = v;
        panel.list_end = v;
    }
    widgetLed(v, 0);
}

static inline void widgetFreeAll()
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->terminate)
            v->api->terminate(v->parent);
        SDL_DestroyTexture(v->texture);
        SDL_FreeSurface(v->surface);
        v = v->next;
    }
}

static inline void widgetRenderAll(SDL_Renderer* rend)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        SDL_UpdateTexture(v->texture, 0, v->surface->pixels, v->surface->pitch);
        SDL_RenderCopy(rend, v->texture, 0, &v->rect);
        v = v->next;
    }
}

static inline void widgetRedrawAll()
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if ((v->api->redraw) && (v->need_redraw)) {
            v->need_redraw = 0;
            v->api->redraw(v->parent);
        }
        v = v->next;
    }
}
static inline void widgetProcessAll(uint32_t clock)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->process)
            v->api->process(v->parent, clock);
        v = v->next;
    }
}
static inline void widgetKeyboardAll(SDL_Event* e)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->keyboard)
            v->api->keyboard(v->parent, e);
        v = v->next;
    }
}
static inline void widgetMouseMoveAll(WidgetTouchData* d, unsigned touch_elements)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->touchMove)
            v->api->touchMove(v->parent, d, touch_elements);
        v = v->next;
    }
}
static inline void widgetMouseClickAll(WidgetTouchData* d)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->touchClick)
            v->api->touchClick(v->parent, d);
        v = v->next;
    }
}
static inline void widgetMouseWheelAll(SDL_Point* pos, int32_t delta)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->mouseWheel)
            v->api->mouseWheel(v->parent, pos, delta);
        v = v->next;
    }
}

static SDL_bool loop = SDL_FALSE;
static SDL_Window* screen;
static SDL_Renderer* rend;
static SDL_Surface* surf;
static SDL_Texture* texture;
static WidgetTouchData touch_data[TOUCHPOINTS_TOTAL]; // touch + mouse
volatile uint32_t fps_counter;

static inline void panelQuit() {
    widgetFreeAll();
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surf);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(screen);
    SDL_Quit();
}

static inline void panelRenderCheck(uint32_t clock) {
    static uint32_t render_time = 0;
    if (clock - render_time < INT32_MAX) {
        fps_counter++;
        SDL_RenderClear(rend);
        SDL_UpdateTexture(texture, 0, surf->pixels, surf->pitch);
        SDL_RenderCopy(rend, texture, 0, 0);
        widgetRedrawAll();
        widgetRenderAll(rend);
        SDL_RenderPresent(rend);
        render_time = SDL_GetTicks() + (1000 / PANEL_FPS);
    }
}

static inline void panelCoreLoop(const SDL_bool userLoopIncluded) {
    SDL_assert(loop);
    SDL_Event event;
    if (0 == SDL_PollEvent(&event)) {
        const uint32_t clock = SDL_GetTicks();
        // process widgets
        widgetProcessAll(clock);
        // process user panel
        if(SDL_TRUE == userLoopIncluded) {
            panelLoop(clock);
        }
        // update framebuffer
        panelRenderCheck(clock);
    } else {
        switch (event.type) {
        // case SDL_POLLSENTINEL:
        //     // end of event queue
        //     break;
        case SDL_MOUSEMOTION:
            DEBUG_PRINTF("\n mousemotion %d, %d", event.motion.x, event.motion.y);
            if (touch_data[TOUCHPOINT_MOUSE].drag) {
                int32_t pval = 0;
                pval += event.motion.xrel;
                int32_t ax = event.motion.yrel;
                if (ax < 0) {
                    ax = -ax;
                }
                if (ax > 3) {
                    pval -= event.motion.yrel * 4;
                }
                touch_data[TOUCHPOINT_MOUSE].drag(touch_data[TOUCHPOINT_MOUSE].instance, pval);
            } else {
                touch_data[TOUCHPOINT_MOUSE].point.x = event.motion.x;
                touch_data[TOUCHPOINT_MOUSE].point.y = event.motion.y;
                widgetMouseMoveAll(touch_data, TOUCHPOINTS_TOTAL);
            }
            break;
        case SDL_FINGERMOTION:
            DEBUG_PRINTF("\n fingermotion %d, %6g, %6g", (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
            for (unsigned i = 0; i < MULTITOUCH_MAX_FINGERS; i++) {
                if (event.tfinger.fingerId == touch_data[TOUCHPOINT_FINGER(i)].finger) {
                    if (touch_data[TOUCHPOINT_FINGER(i)].drag) {
                        int32_t pval = 0;
                        pval += (int)(event.tfinger.dx * (float)(PANEL_SIZE_X * PANEL_SCALE));
                        int32_t ax = (int)(event.tfinger.dy * (float)(PANEL_SIZE_Y * PANEL_SCALE));
                        if ((ax < -1) || (ax > 3)) {
                            pval -= ax * 4;
                        }
                        touch_data[TOUCHPOINT_FINGER(i)].drag(touch_data[TOUCHPOINT_FINGER(i)].instance, pval);
                    } else {
                        touch_data[TOUCHPOINT_FINGER(i)].point.x = (int)(event.tfinger.x * (float)(PANEL_SIZE_X * PANEL_SCALE));
                        touch_data[TOUCHPOINT_FINGER(i)].point.y = (int)(event.tfinger.y * (float)(PANEL_SIZE_Y * PANEL_SCALE));
                        widgetMouseMoveAll(touch_data, TOUCHPOINTS_TOTAL);
                    }
                }
            }
            break;
        // case SDL_DOLLARGESTURE:
        // case SDL_DOLLARRECORD:
        // case SDL_MULTIGESTURE:
        //     DEBUG_PRINTF("\n gesture");
        //     // no gestures!
        //     break;
        case SDL_MOUSEBUTTONUP:
            DEBUG_PRINTF("\n mouse release %d, %d", event.button.x, event.button.y);
            if (touch_data[TOUCHPOINT_MOUSE].drag) {
                touch_data[TOUCHPOINT_MOUSE].drag = 0;
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_WarpMouseInWindow(screen, touch_data[TOUCHPOINT_MOUSE].point.x, touch_data[TOUCHPOINT_MOUSE].point.y);
            } else if (SDL_TRUE == touch_data[TOUCHPOINT_MOUSE].is_pressed) {
                touch_data[TOUCHPOINT_MOUSE].is_pressed = SDL_FALSE;
                widgetMouseMoveAll(touch_data, TOUCHPOINTS_TOTAL);
            }
            break;
        case SDL_FINGERUP:
            DEBUG_PRINTF("\n finger release %d, %6g, %6g", (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
            for (unsigned i = 0; i < MULTITOUCH_MAX_FINGERS; i++) {
                unsigned tpos = TOUCHPOINT_FINGER(i);
                if (event.tfinger.fingerId == touch_data[tpos].finger) {
                    touch_data[tpos].is_pressed = SDL_FALSE;
                    touch_data[tpos].drag = 0;
                    touch_data[tpos].point.x = -1;
                    touch_data[tpos].point.y = -1;
                    touch_data[tpos].finger = 0;
                    widgetMouseMoveAll(touch_data, TOUCHPOINTS_TOTAL);
                }
            }
            // for (unsigned i = 0; i < TOUCHPOINTS_TOTAL; i++) {
            //     DEBUG_PRINTF("\n %d: %d,%d", i, touch_data[i].point.x, touch_data[i].point.y);
            // }
            break;
        case SDL_MOUSEBUTTONDOWN:
            DEBUG_PRINTF("\n mouse press %d, %d", event.button.x, event.button.y);
            widgetMouseClickAll(&touch_data[TOUCHPOINT_MOUSE]);
            if (touch_data[TOUCHPOINT_MOUSE].drag) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                touch_data[TOUCHPOINT_MOUSE].is_pressed = SDL_TRUE;
            }
            break;
        case SDL_FINGERDOWN:
            DEBUG_PRINTF("\n finger press %d, %6g, %6g", (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
            for (unsigned i = 0; i < MULTITOUCH_MAX_FINGERS; i++) {
                unsigned tpos = TOUCHPOINT_FINGER(i);
                if (0 == touch_data[tpos].finger) {
                    touch_data[tpos].finger = event.tfinger.fingerId;
                    touch_data[tpos].point.x = (int)(event.tfinger.x * (float)(PANEL_SIZE_X * PANEL_SCALE));
                    touch_data[tpos].point.y = (int)(event.tfinger.y * (float)(PANEL_SIZE_Y * PANEL_SCALE));
                    touch_data[tpos].drag = 0;
                    widgetMouseClickAll(&touch_data[tpos]);
                    if (0 == touch_data[tpos].drag) {
                        touch_data[tpos].is_pressed = SDL_TRUE;
                    }
                    break;
                }
            }
            // for (unsigned i = 0; i < TOUCHPOINTS_TOTAL; i++) {
            //     DEBUG_PRINTF("\n %d: %d,%d", i, touch_data[i].point.x, touch_data[i].point.y);
            // }
            break;
        case SDL_MOUSEWHEEL:
            DEBUG_PRINTF("\n mouse wheel %d, %d", event.wheel.x, event.wheel.y);
            widgetMouseWheelAll(&touch_data[TOUCHPOINT_MOUSE].point, -16 * event.wheel.y - event.wheel.x);
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            DEBUG_PRINTF("\n keyboard %d", event.key.keysym.sym);
            widgetKeyboardAll(&event);
            if (SDLK_ESCAPE == event.key.keysym.sym) {
                loop = SDL_FALSE;
            }
            break;
        case SDL_QUIT:
            loop = SDL_FALSE;
            break;
        case SDL_WINDOWEVENT:
            break;
        // default: {
        //     static int counter = 0;
        //     DEBUG_PRINTF("\n event %08X, %08X", event.type, counter++);
        // } break;
        } // switch event
    } // event received
    SDL_Delay(1);
}

// for bspDelayMs()
void panelCriticalLoop() {
    panelCoreLoop(SDL_FALSE);
    if (SDL_FALSE == loop) {
        // exit was requested
        panelQuit();
        // __asm volatile("ud2" ::: "memory");
        __builtin_trap();
    }
}

// __attribute__((weak)) 
uint32_t bspGetMs(void) { return SDL_GetTicks(); }

// __attribute__((weak))
void bspDelayMs(const uint32_t time_ms) {
    SDL_assert(loop);
    const uint32_t end = bspGetMs() + time_ms;
    while (end < (volatile uint32_t)bspGetMs()) { panelCriticalLoop(); }
    while (end > (volatile uint32_t)bspGetMs()) { panelCriticalLoop(); }
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    int ret = 0;
    ret = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");

    // // display info
    // for (int i = 0; i < SDL_GetNumVideoDisplays(); i++) {
    //     SDL_DisplayMode m;
    //     SDL_GetDesktopDisplayMode(i, &m);
    //     DEBUG_PRINTF("\nDisplay %d: %d x %d, %d Hz", i, m.w, m.h, m.refresh_rate);
    // }
    // for (int i = 0; i < SDL_GetNumTouchDevices(); i++) { // returns 0 ((
    //     SDL_TouchID t = SDL_GetTouchDevice(i);
    //     DEBUG_PRINTF("\ntouch type %d: %d", i, SDL_GetTouchDeviceType(t));
    // }

    screen = SDL_CreateWindow(PANEL_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        PANEL_SIZE_X * PANEL_SCALE, PANEL_SIZE_Y * PANEL_SCALE, 0);
    rend = SDL_CreateRenderer(screen, -1, SDL_RENDERER_PRESENTVSYNC);
    surf = SDL_CreateRGBSurface(0, PANEL_SIZE_X, PANEL_SIZE_Y, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    texture = SDL_CreateTextureFromSurface(rend, surf);

    panelInit();
    SDL_SetRenderDrawColor(rend,
        (panel.widget_color_panel >> 16) & 0xFF,
        (panel.widget_color_panel >> 8) & 0xFF,
        panel.widget_color_panel & 0xFF,
        0);
    SDL_memset(&touch_data, 0, sizeof(touch_data));
    for (unsigned i = 0; i < TOUCHPOINTS_TOTAL; i++) {
        touch_data[i].point.x = -1, touch_data[i].point.y = -1;
    }

    loop = SDL_TRUE;
    panelConstruct(rend);
    while (loop) {
        panelCoreLoop(SDL_TRUE);
    }
    panelQuit();
    return ret;
}
