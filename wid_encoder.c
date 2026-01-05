#include "wid_encoder.h"
#include "wid_graphics.h"
#include "mbwmidi.h"

__attribute__((weak)) void wEncMidiSend(uint8_t midictrl, uint8_t value)
{
    SDL_assert(midictrl < 128);
    SDL_assert(value < 128);
    MidiMessageT m;
    m.cn = MIDI_CN_LOCALPANEL;
    m.cin = m.miditype = MIDI_CIN_POLYKEYPRESS;
    m.byte2 = midictrl;
    m.byte3 = value;
    midiNonSysexWrite(m);
    // (void)midictrl;
    // (void)value;
}

#define ENCODER_NOTCHES 7
#define ENCODER_SHIFT 16

static void wEncRedraw(void* wid)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    uint32_t color = panel.widget_color_released;
    if (v->pointed) {
        if (v->pointed > 1)
            color = panel.widget_color_pressed;
        else
            color = panel.widget_color_pointed;
    }
    drawOutline(&v->v, color);
    drawLedFill(&v->v, color);
    drawCircle(&v->v, 0, color);
    uint16_t d = v->v.surface->w;
    for (int i = 0; i < ENCODER_NOTCHES; i++) {
        float value = -(uint16_t)(v->value_drag << 4) + (65536 / ENCODER_NOTCHES) * i;
        float angle = (float)value * (1.f / (65536.f)) * (PI_F * 2.f);
        float x = SDL_sinf(angle);
        float y = SDL_cosf(angle);
        int32_t xs = x * (float)(d / 2 - 2) + d / 2;
        int32_t xe = x * (float)(d / 3) + d / 2;
        int32_t ys = y * (float)(d / 2 - 2) + d / 2;
        int32_t ye = y * (float)(d / 3) + d / 2;
        drawLine(&v->v, xs, ys, xe, ye, d, color);
    }
    drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
}
static void wEncProcess(void* wid, uint32_t clock)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    (void)clock;
    int16_t d = v->value_drag - v->value_send;
    int16_t ds = d / ENCODER_SHIFT;
    if (0 != ds) {
        if (ds > 63)
            ds = 63;
        else if (ds < -64)
            ds = -64;
        wEncMidiSend(v->midictrl, ds + 64);
        v->value_send = v->value_drag;
        v->v.need_redraw = 1;
    }
}
static void wEncDrag(void* instance, int32_t delta)
{
    WidgetEnc* v = (WidgetEnc*)instance;
    v->value_drag += delta;
}
static void wEncKeyboard(void* wid, SDL_Event* e)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == (SDL_Keycode)v->keycode_inc) {
            v->value_drag += ENCODER_SHIFT;
            v->v.need_redraw = 1;
        } else if (e->key.keysym.sym == (SDL_Keycode)v->keycode_dec) {
            v->value_drag -= ENCODER_SHIFT;
            v->v.need_redraw = 1;
        }
    }
}
static void wEncMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    uint8_t pointed = 0;
    for (unsigned i = 0; i < touch_elements; i++) {
        if (SDL_PointInRect(&d[i].point, &v->v.rect)) {
            pointed = 1;
        }
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wEncMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        v->pointed = 2;
        d->instance = (void*)v;
        d->drag = wEncDrag;
    }
}
static void wEncMouseWheel(void* wid, SDL_Point* pos, int32_t delta)
{
    WidgetEnc* v = (WidgetEnc*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        wEncDrag(wid, delta);
    }
}
static WidgetApi wEncApi = {
    .redraw = wEncRedraw,
    .process = wEncProcess,
    .keyboard = wEncKeyboard,
    .touchMove = wEncMouseMove,
    .touchClick = wEncMouseClick,
    .mouseWheel = wEncMouseWheel,
    .terminate = 0
};

void wEncInit(
    WidgetEnc* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode_inc,
    SDL_KeyCode keycode_dec,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wEncApi, x, y, panel.widget_unit_size, panel.widget_unit_size, panel.widget_scale, rend);
    v->name = name;
    v->midictrl = midictrl;
    v->kbd = kbd;
    v->keycode_inc = keycode_inc;
    v->keycode_dec = keycode_dec;
    v->pointed = 0;
    v->value_drag = 0;
    v->value_send = 0;
}
