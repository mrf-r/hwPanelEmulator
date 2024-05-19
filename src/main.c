#include <SDL.h>
#include "widget.h"
#include "panel_conf.h"

// --- to be implemented by user
void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t clock);
// ---

uint32_t lcg;
Panel panel;

// #define DEBUG_PRINTF(...)
#define DEBUG_PRINTF printf

static void panelInit()
{
    // TODO: put all this in conf file
    panel.list_start = 0;
    panel.list_end = 0;
    panel.widget_unit_size = PANEL_UNIT_SIZE;
    panel.widget_scale = PANEL_SCALE;
    panel.widget_led_alpha = 0x80;
    panel.widget_fill_alpha = 32;
    panel.widget_color_panel = (125 << 16) | (127 << 8) | 108;
    panel.widget_color_released = 0xFF000000; // normal outline
    panel.widget_color_pointed = 0xFF38400A;
    panel.widget_color_pressed = 0xFF900018; //
    panel.widget_color_helptext = 0xFF483010; // values or keyboard keys info
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
static inline void widgetMouseMoveAll(SDL_Point* pos, uint8_t click)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->mouseMove)
            v->api->mouseMove(v->parent, pos, click);
        v = v->next;
    }
}
static inline void widgetMouseClickAll(SDL_Point* pos, Drag* d)
{
    Widget* v = panel.list_start;
    while (0 != v) {
        if (v->api->mouseClick)
            v->api->mouseClick(v->parent, pos, d);
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

int main(int argc, char* argv[])
{
    int ret = 0;
    ret = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    // // display info
    // for (int i = 0; i < SDL_GetNumVideoDisplays(); i++) {
    //     SDL_DisplayMode m;
    //     SDL_GetDesktopDisplayMode(i, &m);
    //     DEBUG_PRINTF("\nDisplay %d: %d x %d, %d Hz", i, m.w, m.h, m.refresh_rate);
    // }
    // for (int i = 0; i < SDL_GetNumTouchDevices(); i++) {
    //     SDL_TouchID t = SDL_GetTouchDevice(i);
    //     DEBUG_PRINTF("\ntouch type %d: %d", i, SDL_GetTouchDeviceType(t));
    // }

    SDL_Window* screen = SDL_CreateWindow(PANEL_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        PANEL_SIZE_X * PANEL_SCALE, PANEL_SIZE_Y * PANEL_SCALE, 0);
    SDL_Renderer* rend = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface* surf = SDL_CreateRGBSurface(0, PANEL_SIZE_X, PANEL_SIZE_Y, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surf);

    panelInit();
    panelConstruct(rend);

    SDL_SetRenderDrawColor(rend,
        (panel.widget_color_panel >> 16) & 0xFF,
        (panel.widget_color_panel >> 8) & 0xFF,
        panel.widget_color_panel & 0xFF,
        0);

    SDL_bool loop = SDL_TRUE;
    Drag drag = { .drag = 0, .instance = 0 };
    SDL_bool mouse_pressed = SDL_FALSE;
    SDL_Point mouse_last_pos;
    while (loop) {
        SDL_Event event;
        if (0 == SDL_PollEvent(&event)) {

            SDL_Delay(1);

            // process widgets
            uint32_t clock;
            clock = SDL_GetTicks();
            widgetProcessAll(clock);

            // process panel
            panelLoop(clock); // TODO: should we also multiply panel processing?

            // TODO: check if it is time to frame update ???
            // update framebuffer
            SDL_RenderClear(rend);
            SDL_UpdateTexture(texture, 0, surf->pixels, surf->pitch);
            SDL_RenderCopy(rend, texture, 0, 0);

            widgetRedrawAll();
            widgetRenderAll(rend);

            SDL_RenderPresent(rend);
        } else {
            switch (event.type) {
            // case SDL_POLLSENTINEL:
            //     // end of event queue
            //     break;
            case SDL_MOUSEMOTION:
                if (drag.drag) {
                    int32_t pval = 0;
                    pval += event.motion.xrel;
                    int32_t ax = event.motion.yrel;
                    if (ax < 0) {
                        ax = -ax;
                    }
                    if (ax > 3) {
                        pval -= event.motion.yrel * 4;
                    }
                    drag.drag(drag.instance, pval);
                } else {
                    mouse_last_pos.x = event.motion.x;
                    mouse_last_pos.y = event.motion.y;
                    widgetMouseMoveAll(&mouse_last_pos, mouse_pressed);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (drag.drag) {
                    drag.drag = 0;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    SDL_WarpMouseInWindow(screen, mouse_last_pos.x, mouse_last_pos.y);
                } else if (mouse_pressed) {
                    mouse_pressed = SDL_FALSE;
                    widgetMouseMoveAll(&mouse_last_pos, 0);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                widgetMouseClickAll(&mouse_last_pos, &drag);
                if (drag.drag) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    // SDL_ShowCursor(SDL_DISABLE);
                } else {
                    mouse_pressed = SDL_TRUE;
                    // panelMouseMove(&mouse_last_pos, 1);
                }
                break;
            case SDL_MOUSEWHEEL:
                widgetMouseWheelAll(&mouse_last_pos, -16 * event.wheel.y - event.wheel.x);
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                widgetKeyboardAll(&event);
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    loop = SDL_FALSE;
                }
                break;
            case SDL_QUIT: {
                // inform all widgets that we are closing!
                SDL_KeyboardEvent e;
                SDL_memset(&e, 0, sizeof(e));
                e.type = SDL_KEYDOWN;
                e.keysym.sym = SDLK_ESCAPE;
                widgetKeyboardAll(&event);
            }
                loop = SDL_FALSE;
                break;
            case SDL_WINDOWEVENT:
                break;
            default:
                // { static int counter = 0;
                // printf("\n event %08X, %08X", event.type, counter++); }
                break;
            } // switch event
        } // event received
    }

    widgetFreeAll();
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surf);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(screen);
    SDL_Quit();

    return ret;
}