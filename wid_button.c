
#include "wid_button.h"
#include "wid_graphics.h"
#include "mbwmidi.h"

__attribute__((weak)) void wButtonMidiSend(uint8_t midictrl, uint8_t value)
{
    SDL_assert(midictrl < 128);
    SDL_assert(value < 128);
    MidiMessageT m;
    m.cn = MIDI_CN_LOCALPANEL;
    m.cin = m.miditype = value ? MIDI_CIN_NOTEON : MIDI_CIN_NOTEOFF;
    m.byte2 = midictrl;
    m.byte3 = value;
    midiNonSysexWrite(m);
    // (void)midictrl;
    // (void)value;
}

#define BUT_SRC_POINTED 0x01
#define BUT_SRC_MOUSE 0x02
#define BUT_SRC_KEYBD 0x04
// #define BUT_SRC_TOUCH 0x08
// #define BUT_SRC_EXT 0x10
#define BUT_SRC_INT 0x80

static void wButtonRedraw(void* wid)
{
    WidgetButton* v = (WidgetButton*)wid;
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
    drawStringCentered(&v->v, d / 2, d / 2 - 4, v->name, color);
    drawString(&v->v, d / 6, d * 2 / 3, v->kbd, panel.widget_color_helptext);
}
static void wButtonProcess(void* wid, uint32_t clock)
{
    WidgetButton* v = (WidgetButton*)wid;
    (void)clock;
    const uint8_t point_mask = (uint8_t) ~(BUT_SRC_INT | BUT_SRC_POINTED);
    if ((v->pointed & BUT_SRC_INT) && (0 == (v->pointed & point_mask))) {
        v->pointed &= ~BUT_SRC_INT;
        wButtonMidiSend(v->midictrl, 0);
    } else if ((0 == (v->pointed & BUT_SRC_INT)) && (v->pointed & point_mask)) {
        v->pointed |= BUT_SRC_INT;
        wButtonMidiSend(v->midictrl, 1);
    }
}
static void wButtonKeyboard(void* wid, SDL_Event* e)
{
    WidgetButton* v = (WidgetButton*)wid;
    uint8_t pointed = v->pointed;
    if (e->key.keysym.sym == (SDL_Keycode)v->keycode) {
        if (e->type == SDL_KEYDOWN) {
            pointed |= BUT_SRC_KEYBD;
        } else if (e->type == SDL_KEYUP) {
            pointed &= ~BUT_SRC_KEYBD;
        }
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wButtonMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetButton* v = (WidgetButton*)wid;
    uint8_t pointed = v->pointed; // need to preserve keyboard state
    pointed &= ~(BUT_SRC_MOUSE | BUT_SRC_POINTED);
    for (unsigned i = 0; i < touch_elements; i++) {
        if (SDL_PointInRect(&d[i].point, &v->v.rect)) {
            pointed |= BUT_SRC_POINTED;
            if (SDL_TRUE == d[i].is_pressed) {
                pointed |= BUT_SRC_MOUSE;
            }
        }
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}
static void wButtonMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetButton* v = (WidgetButton*)wid;
    (void)d;
    uint8_t pointed = v->pointed;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        pointed |= BUT_SRC_MOUSE | BUT_SRC_POINTED;
    }
    if (v->pointed != pointed) {
        v->v.need_redraw = 1;
    }
    v->pointed = pointed;
}

static WidgetApi wButtonApi = {
    .redraw = wButtonRedraw,
    .process = wButtonProcess,
    .keyboard = wButtonKeyboard,
    .touchMove = wButtonMouseMove,
    .touchClick = wButtonMouseClick,
    .mouseWheel = 0,
    .terminate = 0
};

void wButtonInit(
    WidgetButton* v,
    const char* name,
    const char* kbd,
    SDL_KeyCode keycode,
    uint8_t midictrl,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wButtonApi, x, y, panel.widget_unit_size, panel.widget_unit_size, panel.widget_scale, rend);
    v->name = name;
    v->kbd = kbd;
    v->keycode = keycode;
    v->midictrl = midictrl;
    v->pointed = 0;
}
