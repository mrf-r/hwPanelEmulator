#include "wid_pot.h"
#include "mbwmidi.h"
#include "wid_graphics.h"

#ifndef POT_CTRL_MULT
#define POT_CTRL_MULT 12
#endif

#define POT_MAX (128 * 128)

#define POT_PERIOD_CLK_CYCLES 1 // 1ms = 1kHz
#define POT_BITS_ADC 10 // <16
#define POT_BITS_CLEAN (POT_BITS_ADC - 2) // <POT_BITS_ADC
#if (POT_BITS_ADC >= 14) || (POT_BITS_CLEAN > POT_BITS_ADC)
#error "too high adc resolution"
#endif

static void wPotRedraw(void* wid)
{
    WidgetPot* v = (WidgetPot*)wid;
    uint32_t color = panel.widget_color_released;
    if (v->pointed) {
        if (v->pointed > 1)
            color = panel.widget_color_pressed;
        else
            color = panel.widget_color_pointed;
    }
    drawOutline(&v->v, color);
    drawLedFill(&v->v, color);
    uint16_t d = v->v.surface->w;
    drawCircle(&v->v, 9, color);
    { // draw lock notch
        float value = (float)potGetValue(&v->potdata) * (-1 / (128.f * 128.f));
        float angle = value * (PI_F * 1.5f) - PI_F * 0.25f;
        float x = SDL_sinf(angle);
        float y = SDL_cosf(angle);
        int32_t xs = x * (float)(d / 2 - 2) + d / 2;
        int32_t xe = x * (float)(d / 3) + d / 2;
        int32_t ys = y * (float)(d / 2 - 2) + d / 2;
        int32_t ye = y * (float)(d / 3) + d / 2;
        drawLine(&v->v, xs, ys, xe, ye, d, panel.widget_color_helptext);
    }
    { // draw actual notch
        float value = (float)potGetPhysicalPosition(&v->potdata) * (-1 / (128.f * 128.f));
        float angle = value * (PI_F * 1.5f) - PI_F * 0.25f;
        float x = SDL_sinf(angle);
        float y = SDL_cosf(angle);
        int32_t xs = x * (float)(d / 2 - 2) + d / 2;
        int32_t xe = x * (float)(d / 3) + d / 2;
        int32_t ys = y * (float)(d / 2 - 2) + d / 2;
        int32_t ye = y * (float)(d / 3) + d / 2;
        drawLine(&v->v, xs, ys, xe, ye, d, color);
    }
    drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
    drawU16Centered(&v->v, d / 2, d - 9, potGetValue(&v->potdata), panel.widget_color_helptext);
}
static void wPotProcess(void* wid, uint32_t clock)
{
    WidgetPot* v = (WidgetPot*)wid;
    // TODO: overflow???
    uint16_t pot = 0;
    while ((int32_t)(v->prev_ms - clock) < 0) {
        v->prev_ms += POT_PERIOD_CLK_CYCLES;
        uint32_t noise_flt = widgetRandom();
        int32_t noise_emu = widgetRandom();
        // true value
        int32_t adc = v->analog_src14b;
        // emulated hw + ADC noise
        noise_emu /= (1 << (31 - (14 - POT_BITS_CLEAN)));
        int32_t thr = 1 << (14 - POT_BITS_CLEAN);
        if (adc < thr) {
            noise_emu = noise_emu * adc / thr;
        } else if (adc > (POT_MAX - thr)) {
            noise_emu = noise_emu * (POT_MAX - adc) / thr;
        }
        adc += noise_emu;
        // adc value
        adc = adc >> (14 - POT_BITS_ADC);
        // bringin them back)
        pot = potFilterCompensated(&v->filter, adc, noise_flt, POT_BITS_ADC);
    }
    MidiMessageT m;
    m.cn = MIDI_CN_LOCALPANEL;
    m.cin = m.miditype = MIDI_CIN_CONTROLCHANGE;
    m.byte2 = v->midictrl;
    if (pot != potGetPhysicalPosition(&v->potdata)) {
        v->v.need_redraw = 1;
    }
    potProcessLocalValueWithMidiSend(&v->potdata, m, pot);
    if (potGetValue(&v->potdata) != v->prev_lock) {
        v->v.need_redraw = 1;
        v->prev_lock = potGetValue(&v->potdata);
    }
}

static void wPotDrag(void* instance, int32_t delta)
{
    if (delta) {
        delta = delta * POT_CTRL_MULT;
        // make low bits static random
        int32_t lowbits = (((int32_t)widgetRandom() / 65536) * POT_CTRL_MULT) / 65536;
        // delta = delta + (delta < 0 ? -lowbits : lowbits);
        delta = delta + lowbits;
        WidgetPot* v = (WidgetPot*)instance;
        int32_t value = v->analog_src14b + delta;
        if (value < 0) {
            value = 0;
        } else if (value > POT_MAX) {
            value = POT_MAX;
        }
        v->analog_src14b = value;
    }
}
static void wPotMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetPot* v = (WidgetPot*)wid;
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
static void wPotMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetPot* v = (WidgetPot*)wid;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        v->pointed = 2;
        v->v.need_redraw = 1;
        d->instance = (void*)v;
        d->drag = wPotDrag;
    }
}
static void wPotMouseWheel(void* wid, SDL_Point* pos, int32_t delta)
{
    WidgetPot* v = (WidgetPot*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        wPotDrag(wid, delta);
    }
}
static WidgetApi wPotApi = {
    .redraw = wPotRedraw,
    .process = wPotProcess,
    .keyboard = 0,
    .touchMove = wPotMouseMove,
    .touchClick = wPotMouseClick,
    .mouseWheel = wPotMouseWheel,
    .terminate = 0
};

void wPotInit(
    WidgetPot* v,
    const char* name,
    const uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    SDL_assert(midictrl < 0x20); // only double cc for primary source
    widgetInit(&v->v, (void*)v, &wPotApi, x, y, panel.widget_unit_size, panel.widget_unit_size, panel.widget_scale, rend);
    v->name = name;
    v->midictrl = midictrl;
    potInit(&v->potdata, 0, 0); // TODO: set initial from eeprom load
    v->pointed = 0;
    v->analog_src14b = 0;
    v->filter = 0;
    v->prev_ms = SDL_GetTicks();
}
