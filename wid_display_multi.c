#include "wid_display_multi.h"
#include "wid_graphics.h"

static void wDispMultiRedraw(void* wid)
{
    WidgetDisplayMulti* v = (WidgetDisplayMulti*)wid;
    drawOutline(&v->v, panel.widget_color_released);
}

static WidgetApi wDispMultiApi = {
    .redraw = wDispMultiRedraw,
    .process = 0,
    .keyboard = 0,
    .touchMove = 0,
    .touchClick = 0,
    .mouseWheel = 0,
    .terminate = 0
};

void wDisplayMultiInit(
    WidgetDisplayMulti* v,
    MglDisplay* disp,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend)
{
    SDL_memset(disp->context, 0, sizeof(MglDispContext));
    SDL_memset(v, 0, sizeof(WidgetDisplayMulti));
    widgetInit(&v->v, (void*)v, &wDispMultiApi, x, y, disp->size_x + 2, disp->size_y + 2, scale, rend);
    v->disp = disp;
    drawOutline(&v->v, panel.widget_color_released);
    mgsDisplay(disp);
    mgsFont(&_5x7mod);
    MglColor back = { .wrd = 0xFF000000 };
    mgsBackColor(back);
    mgsAlign(MGL_ALIGN_LEFT);
}
