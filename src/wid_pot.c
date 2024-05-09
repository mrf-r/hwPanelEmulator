#include "wid_pot.h"
#include "wid_graphics.h"

// __attribute__((weak)) static void wPotMidiSend(uint8_t midictrl, uint16_t value)
// {
//     (void)midictrl;
//     (void)value;
// }
void wPotMidiSend(uint8_t midictrl, uint16_t value);

#define POT_PERIOD_MS 1 // 1ms = 1kHz ?
#define POT_BITS_ADC 10 // <16
#define POT_BITS_CLEAN 7 // <POT_BITS_ADC
#define POT_LOCK_THRSH (128 * 2)
#define POT_MAX ((1 << POT_BITS_ADC) - 1)
#if (POT_BITS_ADC >= 16) || (POT_BITS_CLEAN > POT_BITS_ADC)
#error "too high adc resolution"
#endif
#define AFILTER_CF1 4 // 0 < cf < 16
#define AFILTER_CF2 0 // 0 < cf < 16

static void wPotRedraw(void* wid)
{
    WidgetPot* v = (WidgetPot*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
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
        if ((v->state == POT_STATE_LOCK_LOW) || (v->state == POT_STATE_LOCK_HIGH)) {
            // draw locked notch
            float value = (float)v->lock_value * (-1 / (128.f * 128.f));
            float angle = value * (PI_F * 1.5f) - PI_F * 0.25f;
            float x = SDL_sinf(angle);
            float y = SDL_cosf(angle);
            int32_t xs = x * (float)(d / 2 - 2) + d / 2;
            int32_t xe = x * (float)(d / 3) + d / 2;
            int32_t ys = y * (float)(d / 2 - 2) + d / 2;
            int32_t ye = y * (float)(d / 3) + d / 2;
            drawLine(&v->v, xs, ys, xe, ye, d, panel.widget_color_helptext);
        }
        { // draw normal notch
            float value = (float)v->output * (-1 / (128.f * 128.f));
            float angle = value * (PI_F * 1.5f) - PI_F * 0.25f;
            float x = SDL_sinf(angle);
            float y = SDL_cosf(angle);
            int32_t xs = x * (float)(d / 2 - 2) + d / 2;
            int32_t xe = x * (float)(d / 3) + d / 2;
            int32_t ys = y * (float)(d / 2 - 2) + d / 2;
            int32_t ye = y * (float)(d / 3) + d / 2;
            drawLine(&v->v, xs, ys, xe, ye, d, v->state == POT_STATE_LOCK_INSIDE ? panel.widget_color_helptext : color);
        }
        drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
        drawU16Centered(&v->v, d / 2, d - 9, v->output, panel.widget_color_helptext);
    }
}
static void wPotProcess(void* wid, uint32_t ms)
{
    WidgetPot* v = (WidgetPot*)wid;
    // TODO ms!!
    // SDL_assert((int32_t)(v->prev_ms - ms) > -40);
    // while ((int32_t)(v->prev_ms - ms) < 0) {
    if (1) {
        v->prev_ms += POT_PERIOD_MS;
        uint32_t noise_flt = widgetRandom();
        int32_t noise_emu = widgetRandom();

        // emulated ADC noise
        int32_t adc = v->adc_source;
        adc += noise_emu / (1 << (31 - (POT_BITS_ADC - POT_BITS_CLEAN)));
        if (adc < 0) {
            adc = 0;
        } else if (adc > POT_MAX) {
            adc = POT_MAX;
        }

        // adaptive filter
        // AFILTER_NLCF = ~5 (0..16)
        // AFILTER_LCF = ~0 (0..16)
        int32_t aflt = v->filter;
        int32_t in = adc * (1 << (31 - POT_BITS_ADC));
        in |= noise_flt >> (POT_BITS_ADC + 1);
        int32_t delta = in - aflt;
        int32_t cut = delta / (1 << (16 + AFILTER_CF1));
        cut = cut * cut;
        if (cut > (1 << (16 - AFILTER_CF2))) {
            cut = (1 << (16 - AFILTER_CF2));
        }
        aflt += delta / (1 << (16 + AFILTER_CF2)) * cut;
        v->filter = aflt;

        uint16_t out_prev = v->output;
        uint16_t out = aflt / (1 << (31 - 14));
        v->output = out;
        if (out != out_prev) {
            v->v.need_redraw = 1;
            switch (v->state) {
            case POT_STATE_NORMAL: {
                wPotMidiSend(v->midictrl, v->output);
            } break;
            case POT_STATE_LOCK_INSIDE: {
                int16_t delta = v->output - v->lock_value;
                if (delta < 0) {
                    delta = -delta;
                }
                if (delta > POT_LOCK_THRSH) {
                    v->state = POT_STATE_NORMAL;
                    wPotMidiSend(v->midictrl, v->output);
                }
            } break;
            case POT_STATE_LOCK_LOW: {
                if (v->output > v->lock_value) {
                    v->state = POT_STATE_NORMAL;
                    wPotMidiSend(v->midictrl, v->output);
                }
            } break;
            case POT_STATE_LOCK_HIGH: {
                if (v->output < v->lock_value) {
                    v->state = POT_STATE_NORMAL;
                    wPotMidiSend(v->midictrl, v->output);
                }
            } break;
            default:
                break;
            }
        }
    }
}
static void wPotKeyboard(void* wid, SDL_Event* e)
{
    (void)wid;
    (void)e;
}
static void wPotDrag(void* instance, int32_t delta)
{
    WidgetPot* v = (WidgetPot*)instance;
    int32_t value = v->adc_source + delta;
    if (value < 0) {
        value = 0;
    } else if (value > POT_MAX) {
        value = POT_MAX;
    }
    v->adc_source = value;
}
static void wPotMouseMove(void* wid, SDL_Point* pos, uint8_t click)
{
    WidgetPot* v = (WidgetPot*)wid;
    (void)click;
    uint8_t pointed = v->pointed;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        pointed = 1;
    } else {
        pointed = 0;
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wPotMouseClick(void* wid, SDL_Point* pos, Drag* d)
{
    WidgetPot* v = (WidgetPot*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        v->pointed = 2;
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
    .keyboard = wPotKeyboard,
    .mouseMove = wPotMouseMove,
    .mouseClick = wPotMouseClick,
    .mouseWheel = wPotMouseWheel
};

void wPotInit(WidgetPot* v, const char* name, const uint8_t midictrl, uint16_t x, uint16_t y, SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wPotApi, x, y, panel.widget_unit_size, panel.widget_unit_size, panel.widget_scale, rend);
    v->name = name;
    v->midictrl = midictrl;
    v->pointed = 0;
    v->state = POT_STATE_NORMAL;
    v->lock_value = 0;
    v->adc_source = 0;
    v->filter = 0;
    v->output = 0;
    v->prev_ms = SDL_GetTicks();
}

void wPotLock(WidgetPot* v, uint8_t midictrl, uint8_t is_new_val, uint16_t value)
{
    if (v->midictrl == midictrl) {
        if (is_new_val) {
            v->lock_value = value;
            if (value < v->output) {
                v->state = POT_STATE_LOCK_LOW;
            } else {
                v->state = POT_STATE_LOCK_HIGH;
            }
        } else {
            v->lock_value = v->output;
            v->state = POT_STATE_LOCK_INSIDE;
        }
    }
}
