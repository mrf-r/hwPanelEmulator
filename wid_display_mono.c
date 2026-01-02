#include "wid_display_mono.h"
#include "wid_graphics.h"
#include "panel_conf.h"

// display
#ifndef DISPM_COLOR_BACK
#define DISPM_COLOR_BACK 0x000000FF // blue inverted
#endif
#ifndef DISPM_COLOR_PIXEL
#define DISPM_COLOR_PIXEL 0xFFFFFF
#endif
// #define DISPM_COLOR_BACK 0x0040C000 // green standart
// #define DISPM_COLOR_PIXEL 0x0
#define DISPM_ALPHA 0xA0
#define DISPM_GAP 2
extern const MglFont _5x7mod;

static void wDispMonoRedraw(void* wid)
{
    WidgetDisplayMono* v = (WidgetDisplayMono*)wid;
    drawOutline(&v->v, panel.widget_color_released);

    MglColor led = { .wrd = (v->v.led & 0x00FFFFFF) | (DISPM_ALPHA << 24) };
    MglColor pix = { .wrd = (v->color_pix & 0x00FFFFFF) | (DISPM_ALPHA << 24) };
    pix = mgAlphablend(0x60, pix, led);
    pix.alpha = DISPM_ALPHA;
    uint32_t* framebuffer = (uint32_t*)v->v.surface->pixels;
    // draw gap pixels
    uint8_t gap = DISPM_GAP + 1;
    for (int i = 0; i < DISPM_GAP; i++) {
        for (int x = 1; x < v->v.surface->w - 1; x++) {
            framebuffer[v->v.surface->w * (1 + i) + x] = led.wrd;
            framebuffer[v->v.surface->w * (v->v.surface->h - i - 2) + x] = led.wrd;
        }
        for (int y = 1; y < v->v.surface->h - 1; y++) {
            framebuffer[v->v.surface->w * y + i + 1] = led.wrd;
            framebuffer[v->v.surface->w * y + v->v.surface->w - i - 2] = led.wrd;
        }
    }
    // draw actual pixels
    for (int y = 0; y < v->disp->size_y; y++) {
        for (int x = 0; x < v->disp->size_x; x++) {
            uint32_t fbpos_mono = y / 8 * v->disp->size_x + x;
            uint32_t fbshift_mono = y & 0x7;
            uint32_t p = (v->framebuffer[fbpos_mono] >> fbshift_mono) & 0x1;
            p = (p == 1) ? pix.wrd : led.wrd;
            framebuffer[(y + gap) * v->v.surface->w + (x + gap)] = p;
        }
    }
}

static void wDispMonoMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetDisplayMono* v = (WidgetDisplayMono*)wid;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        // TODO: add timestamp to filename YYMMDD-HHMMSS-MSEC-screenshot.bmp
#ifdef DISPLAY_SCREENSHOT_DISABLE_ALPHA
        // disable alpha before saving
        uint32_t* framebuffer = (uint32_t*)v->v.surface->pixels;
        unsigned len = v->v.surface->h * v->v.surface->w;
        for (unsigned i = 0; i < len; i++) {
            framebuffer[i] = framebuffer[i] | 0xFF000000;
        }
        v->v.need_redraw = 1;
#endif // DISPLAY_SCREENSHOT_DISABLE_ALPHA
        SDL_SaveBMP(v->v.surface, "screenshot2.bmp");
    }
}
static WidgetApi wDispMonoApi = {
    .redraw = wDispMonoRedraw,
    .process = 0,
    .keyboard = 0,
    .touchMove = 0,
    .touchClick = wDispMonoMouseClick,
    .mouseWheel = 0,
    .terminate = 0
};

void wDisplayMonoInit(
    WidgetDisplayMono* v,
    MglDisplay* disp,
    uint8_t* framebuffer,
    uint16_t x,
    uint16_t y,
    uint8_t scale,
    SDL_Renderer* rend)
{
    SDL_memset(disp->context, 0, sizeof(MglDispContext));
    SDL_memset(v, 0, sizeof(WidgetDisplayMono));
    uint8_t gap = DISPM_GAP + 1;
    widgetInit(&v->v, (void*)v, &wDispMonoApi, x, y, disp->size_x + 2 * gap, disp->size_y + 2 * gap, scale, rend);
    v->disp = disp;
    v->framebuffer = framebuffer;
    v->color_pix = DISPM_COLOR_PIXEL;
    widgetLed(&v->v, DISPM_COLOR_BACK);
    mgsDisplay(disp);
    mgsFont(&_5x7mod);
    mgsBackColor(COLOR_OFF);
    mgsAlign(MGL_ALIGN_LEFT);
}
