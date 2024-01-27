// TODO: rename to main.c

#include <SDL.h>
#include "widget.h"
#include "userpanel.h"

#ifndef PANEL_PROCESSRATEMUL
#define PANEL_PROCESSRATEMUL 1
#endif

Widget* list_start = 0;
Widget* list_end = 0;

uint8_t widget_unit_size = 46;
uint8_t widget_scale = 2;
uint8_t widget_led_alpha = 0x80;
uint8_t widget_fill_alpha = 32;
uint32_t widget_color_panel = (125 << 16) | (127 << 8) | 108;
uint32_t widget_color_released = 0xFF000000; // normal outline
uint32_t widget_color_pointed = 0xFF38400A;
uint32_t widget_color_pressed = 0xFF900018; //
uint32_t widget_color_helptext = 0xFF483010; // values or keyboard keys info

#define DEBUG_PRINTF(...)
// #define DEBUG_PRINTF printf

int main(int argc, char* argv[])
{
    int ret = 0;
    ret = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    // display info
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

    panelConstruct(rend);

    SDL_SetRenderDrawColor(rend,
        (widget_color_panel >> 16) & 0xFF,
        (widget_color_panel >> 8) & 0xFF,
        widget_color_panel & 0xFF,
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
            uint32_t ms;
            for (int i = 0; i < PANEL_PROCESSRATEMUL; i++) {
                ms = SDL_GetTicks(); // TODO: dumb, but for now it is probably ok
                widgetProcessAll(ms);
            }
            // process panel
            panelLoop(ms); // TODO: should we also multiply panel processing?

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