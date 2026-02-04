
#include "wid_piano.h"
#include "wid_graphics.h"
#include "mbwmidi.h"

/*
oh, it's a whole another panel in a widget...
TODO:
- hold (via caps?)
- last key oct up (A)
- last key oct dwn (F)
- ? (K)
- ? (")
*/

#define NUM_WKEYS 10
#define KEY_WIDTH 7
#define WKEY_WIDTH 12
#define TOP_HEIGHT 8
#define KEY_HEIGHT 8

#define BMP_CHANNEL 0x80000000
#define BMP_OCTAVE 0x40000000
#define BMP_VELO 0x20000000
#define BMP_SHIFT 0x10000000

typedef struct {
    uint8_t is_black;
    uint8_t top_xs;
    uint8_t top_xe;
    uint8_t bot_xs;
    uint8_t bot_xe;
} keyCoordinates;

static const uint8_t keymap_white[] = {
    0, 2, 4, 5, 7, 9, 11, 12, 14, 16
};

#define KCBLACK(xb) {1, KEY_WIDTH * xb, KEY_WIDTH * (xb + 1), 0, 0}
#define KCWHITE(xb, xw) {0, \
    KEY_WIDTH * xb, \
    KEY_WIDTH * (xb + 1) + (xb == 16 ? 1 : 0), \
    WKEY_WIDTH * xw + ((xw == 3) || (xw == 4) ? -1 : 0), \
    WKEY_WIDTH * (xw + 1) + ((xw + 1 == 3) || (xw + 1 == 4) ? -1 : 0)}

static const keyCoordinates key_coords[WID_PIANO_NUM_KEYS] = {
    KCWHITE(0, 0),
    KCBLACK(1),
    KCWHITE(2, 1),
    KCBLACK(3),
    KCWHITE(4, 2),
    KCWHITE(5, 3),
    KCBLACK(6),
    KCWHITE(7, 4),
    KCBLACK(8),
    KCWHITE(9, 5),
    KCBLACK(10),
    KCWHITE(11, 6),
    KCWHITE(12, 7),
    KCBLACK(13),
    KCWHITE(14, 8),
    KCBLACK(15),
    KCWHITE(16, 9),
};

__attribute__((weak)) void wPianoMidiSend(WidgetPiano *v, uint8_t key)
{
    MidiMessageT m;
    if ((v->press_int_bmp >> key) & 0x1) {
        uint8_t note = key + v->octave * 12;
        if (v->press_int_bmp & BMP_SHIFT) {
            note += 12;
        }
        m.cn = MIDI_CN_USB_DEVICE;
        m.cin = m.miditype = MIDI_CIN_NOTEON;
        m.midichannel = v->channel;
        m.byte2 = note;
        m.byte3 = v->velocity;
        v->keys[key].channel = v->channel;
        v->keys[key].note = note;
        // printf("noteOn: %02X, %02X, %02X\r\n", m.byte1, m.byte2, m.byte3);
    } else {
        m.cn = MIDI_CN_USB_DEVICE;
        m.cin = m.miditype = MIDI_CIN_NOTEOFF;
        m.midichannel = v->keys[key].channel;
        m.byte2 = v->keys[key].note;
        m.byte3 = 0;
        // printf("noteOff: %02X, %02X, %02X\r\n", m.byte1, m.byte2, m.byte3);
    }
    midiNonSysexWrite(m);
}

static uint32_t colorValue(uint32_t color, int8_t value) {
    uint32_t result = 0;
    int32_t v;
    v = (color & 0xFF) * (value + 128) / 128;
    if (v > 255) {
        v = 255;
    } else if (v < 0) {
        v = 0;
    }
    result |= v;
    v = ((color >> 8) & 0xFF) * (value + 128) / 128;
    if (v > 255) {
        v = 255;
    } else if (v < 0) {
        v = 0;
    }
    result |= v<<8;
    v = ((color >> 16) & 0xFF) * (value + 128) / 128;
    if (v > 255) {
        v = 255;
    } else if (v < 0) {
        v = 0;
    }
    result |= v<<16;
    return result;
}

static void wPianoRedraw(void* wid)
{
    WidgetPiano* v = (WidgetPiano*)wid;
    uint32_t* bmp = (uint32_t*)v->v.surface->pixels;
    const int xe = v->v.surface->w;
    const int ye = v->v.surface->h;

    for (int k = 0; k < WID_PIANO_NUM_KEYS; k++) {
        if ((v->update_bmp >> k) & 0x1) {
            uint32_t color = panel.widget_color_released;
            if (((v->press_int_bmp | v->press_ext_bmp) >> k) & 0x1) {
                color = panel.widget_color_pressed;
            } else if ((v->pointed_bmp >> k) & 0x1) {
                color = panel.widget_color_pointed;
            }
            // printf("redraw: %02X, %02X, %08X\r\n", k, 0, color);
            const int kys = TOP_HEIGHT;
            const int kym = TOP_HEIGHT + KEY_HEIGHT * v->scale;
            const int kye = TOP_HEIGHT + KEY_HEIGHT * 2 * v->scale - 1;
            const int txs = key_coords[k].top_xs * v->scale;
            const int txe = key_coords[k].top_xe * v->scale;
            const int bxs = key_coords[k].bot_xs * v->scale;
            const int bxe = key_coords[k].bot_xe * v->scale;
            // draw
            if (key_coords[k].is_black) {
                // black key draw - one filled rectangle
                uint32_t color_b = colorValue(color, -20);
                // // top and bottom lines
                // for (int x = txs; x < txe; x++) {
                //     bmp[kys * xe + x] = color_b;
                //     bmp[(kym - 1) * xe + x] = color_b;
                // }
                // // side lines
                // for (int y = kys + 1; y < kym; y++) {
                //     bmp[y * xe + txs] = color_b;
                //     bmp[y * xe + txe - 1] = color_b;
                // }
                // fill 
                for (int y = kys + 1; y < kym - 1; y++) {
                    for (int x = txs + 1; x < txe - 1; x++) {
                        bmp[y * xe + x] = (color_b & 0x00FFFFFF) | (panel.widget_fill_alpha << 24);
                    }
                }
            } else {
                // white key draw - two rectangles
                uint32_t color_w = colorValue(color, 20);
                // // horiz lines
                // for (int x = bxs; x < bxe; x++) {
                //     if ((x >= txs) && (x < txe)) {
                //         bmp[kys * xe + x] = color_w;
                //     }
                //     if (((x >= bxs) && (x <= txs)) || ((x >= txe - 1) && (x <= bxe))) {
                //         bmp[kym * xe + x] = color_w;
                //     }
                //     bmp[(ye - 1) * xe + x] = color_w;
                // }
                // // vert lines
                // for (int y = kys + 1; y < kym; y++) {
                //     bmp[y * xe + txs] = color_w;
                //     bmp[y * xe + txe - 1] = color_w;
                // }
                // for (int y = kym + 1; y < kye; y++) {
                //     bmp[y * xe + bxs] = color_w;
                //     bmp[y * xe + bxe - 1] = color_w;
                // }
                // fill 
                for (int y = kys + 1; y < kym + 1; y++) {
                    for (int x = txs + 1; x < txe - 1; x++) {
                        bmp[y * xe + x] = (color_w & 0x00FFFFFF) | (panel.widget_fill_alpha << 24);
                    }
                }
                for (int y = kym + 1; y < kye - 1; y++) {
                    for (int x = bxs + 1; x < bxe - 1; x++) {
                        bmp[y * xe + x] = (color_w & 0x00FFFFFF) | (panel.widget_fill_alpha << 24);
                    }
                }
            }
        }
    }
    uint32_t color = panel.widget_color_helptext;
    if (v->update_bmp & BMP_CHANNEL) {
        const SDL_Rect r = {
            0,
            0,
            xe / 3,
            TOP_HEIGHT
        };
        SDL_FillRect(v->v.surface, &r, 0);
        int x = 0;
        drawString(&v->v, 0, 0, "ch:", color);
        drawU16Centered(&v->v, x + 16, 0, v->channel, color);
    }
    if (v->update_bmp & BMP_OCTAVE) {
        const SDL_Rect r = {
            0 + xe / 3,
            0,
            xe / 3,
            TOP_HEIGHT
        };
        SDL_FillRect(v->v.surface, &r, 0);
        int x = xe / 3;
        drawString(&v->v, x, 0, "oct:", color);
        drawU16Centered(&v->v, x + 16, 0, v->octave, color);
    }
    if (v->update_bmp & BMP_VELO) {
        const SDL_Rect r = {
            xe * 2 / 3,
            0,
            xe / 3,
            TOP_HEIGHT
        };
        SDL_FillRect(v->v.surface, &r, 0);
        int x = xe * 2 / 3;
        drawString(&v->v, x, 0, "v:", color);
        drawU16Centered(&v->v, x + 16, 0, v->velocity, color);
    }
    v->update_bmp = 0;
}

static void wPianoProcess(void* wid, uint32_t clock)
{
    WidgetPiano* v = (WidgetPiano*)wid;
    (void)clock;
    uint32_t pressed_bmp = v->press_keyb_bmp | v->press_mouse_bmp;
    uint32_t pressed_upd_bmp = pressed_bmp ^ v->press_int_bmp;
    v->press_int_bmp = pressed_bmp;
    v->update_bmp |= pressed_upd_bmp;
    if (v->update_bmp) {
        v->v.need_redraw = 1;
    }
    for (int i = 0; i < WID_PIANO_NUM_KEYS; i++) {
        if ((pressed_upd_bmp >> i) & 0x1) {
            wPianoMidiSend(v, i);
        }
    }
}

static void wPianoKeyboard(void* wid, SDL_Event* e)
{
    const SDL_Keycode keymap[WID_PIANO_NUM_KEYS] = {
        SDLK_z, SDLK_s, SDLK_x, SDLK_d, SDLK_c, SDLK_v, SDLK_g, SDLK_b, SDLK_h, SDLK_n, SDLK_j, SDLK_m, SDLK_COMMA, SDLK_l, SDLK_PERIOD, SDLK_SEMICOLON, SDLK_SLASH
    };
    WidgetPiano* v = (WidgetPiano*)wid;
    if (SDL_KEYDOWN == e->type) {
        for (int i = 0; i < WID_PIANO_NUM_KEYS; i++) {
            if (keymap[i] == e->key.keysym.sym) {
                v->press_keyb_bmp |= 1<<i;
            }
        }
        if ((SDLK_LSHIFT == e->key.keysym.sym) || (SDLK_RSHIFT == e->key.keysym.sym)) {
            v->press_keyb_bmp |= BMP_SHIFT;
        }
    } else if (SDL_KEYUP == e->type) {
        for (int i = 0; i < WID_PIANO_NUM_KEYS; i++) {
            if (keymap[i] == e->key.keysym.sym) {
                v->press_keyb_bmp &= ~(1<<i);
            }
        }
        if ((SDLK_LSHIFT == e->key.keysym.sym) || (SDLK_RSHIFT == e->key.keysym.sym)) {
            v->press_keyb_bmp &= ~BMP_SHIFT;
        }
    }
}

static void wPianoMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetPiano* v = (WidgetPiano*)wid;
    uint32_t pointed = 0;
    uint32_t pressed = 0;
    for (unsigned i = 0; i < touch_elements; i++) {
        const SDL_Rect topline = {
            v->v.rect.x,
            v->v.rect.y + TOP_HEIGHT * panel.widget_scale,
            v->v.rect.w,
            KEY_HEIGHT * v->scale * panel.widget_scale
        };
        const SDL_Rect botline = {
            v->v.rect.x,
            v->v.rect.y + (TOP_HEIGHT + KEY_HEIGHT * v->scale) * panel.widget_scale,
            v->v.rect.w,
            KEY_HEIGHT * v->scale * panel.widget_scale
        };
        if (SDL_PointInRect(&d[i].point, &topline)) {
            int pos = (d[i].point.x - v->v.rect.x) / (KEY_WIDTH * v->scale * panel.widget_scale);
            if (pos > WID_PIANO_NUM_KEYS - 1) {
                pos = WID_PIANO_NUM_KEYS - 1;
            }
            pointed |= 1 << pos;
            if (SDL_TRUE == d[i].is_pressed) {
                pressed |= 1 << pos;
            }
        } else if (SDL_PointInRect(&d[i].point, &botline)) {
            int pos = (d[i].point.x - v->v.rect.x) / (WKEY_WIDTH * v->scale * panel.widget_scale);
            pointed |= 1 << keymap_white[pos];
            if (SDL_TRUE == d[i].is_pressed) {
                pressed |= 1 << keymap_white[pos];
            }
        }
    }
    int upd = pointed ^ v->pointed_bmp;
    v->pointed_bmp = pointed;
    v->update_bmp |= upd;
    v->press_mouse_bmp = pressed;
    if (upd) {
        v->v.need_redraw = 1;
    }
}

#ifndef WPIANO_DRAG_SCALE
#define WPIANO_DRAG_SCALE 16
#endif
#ifndef WPIANO_MAX_OCT
#define WPIANO_MAX_OCT 8
#endif

static void wPianoChDrag(void* instance, int32_t delta)
{
    WidgetPiano* v = (WidgetPiano*)instance;
    int32_t ch = v->channel + delta / WPIANO_DRAG_SCALE;
    // while (ch >= 16) {
    //     ch -= 16;
    // }
    // while (ch < 0) {
    //     ch += 16;
    // }
    ch &= 0xF;
    v->channel = ch;
    v->update_bmp |= BMP_CHANNEL;
    v->v.need_redraw = 1;
}
static void wPianoOctDrag(void* instance, int32_t delta)
{
    WidgetPiano* v = (WidgetPiano*)instance;
    int32_t oct = v->octave + delta / WPIANO_DRAG_SCALE;
    if (oct > WPIANO_MAX_OCT) {
        oct = WPIANO_MAX_OCT;
    }
    if (oct < 0) {
        oct = 0;
    }
    v->octave = oct;
    v->update_bmp |= BMP_OCTAVE;
    v->v.need_redraw = 1;
}
static void wPianoVeloDrag(void* instance, int32_t delta)
{
    WidgetPiano* v = (WidgetPiano*)instance;
    int32_t velo = v->velocity + delta / WPIANO_DRAG_SCALE;
    if (velo > 127) {
        velo = 127;
    }
    if (velo < 1) {
        velo = 1;
    }
    v->velocity = velo;
    v->update_bmp |= BMP_VELO;
    v->v.need_redraw = 1;
}

static void wPianoMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetPiano* v = (WidgetPiano*)wid;
    const SDL_Rect topline = {
        v->v.rect.x,
        v->v.rect.y + TOP_HEIGHT * panel.widget_scale,
        v->v.rect.w,
        KEY_HEIGHT * v->scale * panel.widget_scale
    };
    const SDL_Rect botline = {
        v->v.rect.x,
        v->v.rect.y + (TOP_HEIGHT + KEY_HEIGHT * v->scale) * panel.widget_scale,
        v->v.rect.w,
        KEY_HEIGHT * v->scale * panel.widget_scale
    };
    const SDL_Rect rect_ch = {
        v->v.rect.x,
        v->v.rect.y,
        v->v.rect.w / 3,
        TOP_HEIGHT * v->scale * panel.widget_scale
    };
    const SDL_Rect rect_oct = {
        v->v.rect.x + v->v.rect.w / 3,
        v->v.rect.y,
        v->v.rect.w / 3,
        TOP_HEIGHT * v->scale * panel.widget_scale
    };
    const SDL_Rect rect_velo = {
        v->v.rect.x + v->v.rect.w * 2 / 3,
        v->v.rect.y,
        v->v.rect.w / 3,
        TOP_HEIGHT * v->scale * panel.widget_scale
    };
    
    if (SDL_PointInRect(&d->point, &topline)) {
        int pos = (d->point.x - v->v.rect.x) / (KEY_WIDTH * v->scale * panel.widget_scale);
        if (pos > WID_PIANO_NUM_KEYS - 1) {
            pos = WID_PIANO_NUM_KEYS - 1;
        }
        v->press_mouse_bmp |= 1 << pos;
    } else if (SDL_PointInRect(&d->point, &botline)) {
        int pos = (d->point.x - v->v.rect.x) / (WKEY_WIDTH * v->scale * panel.widget_scale);
        v->press_mouse_bmp |= 1 << keymap_white[pos];
    } else if (SDL_PointInRect(&d->point, &rect_ch)) {
        v->press_mouse_bmp |= BMP_CHANNEL;
        d->instance = (void*)v;
        d->drag = wPianoChDrag;
    } else if (SDL_PointInRect(&d->point, &rect_oct)) {
        v->press_mouse_bmp |= BMP_OCTAVE;
        d->instance = (void*)v;
        d->drag = wPianoOctDrag;
    } else if (SDL_PointInRect(&d->point, &rect_velo)) {
        v->press_mouse_bmp |= BMP_VELO;
        d->instance = (void*)v;
        d->drag = wPianoVeloDrag;
    }
}
static void wPianoMouseWheel(void* wid, SDL_Point* pos, int32_t delta)
{
    WidgetPiano* v = (WidgetPiano*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
        const SDL_Rect rect_ch = {
            v->v.rect.x,
            v->v.rect.y,
            v->v.rect.w / 3,
            TOP_HEIGHT * v->scale * panel.widget_scale
        };
        const SDL_Rect rect_oct = {
            v->v.rect.x + v->v.rect.w / 3,
            v->v.rect.y,
            v->v.rect.w / 3,
            TOP_HEIGHT * v->scale * panel.widget_scale
        };
        if (SDL_PointInRect(pos, &rect_ch)) {
            wPianoChDrag(wid, delta);
        } else if (SDL_PointInRect(pos, &rect_oct)) {
            wPianoOctDrag(wid, delta);
        } else {
            wPianoVeloDrag(wid, delta);
        }
    }
}

static WidgetApi wPianoApi = {
    .redraw = wPianoRedraw,
    .process = wPianoProcess,
    .keyboard = wPianoKeyboard,
    .touchMove = wPianoMouseMove,
    .touchClick = wPianoMouseClick,
    .mouseWheel = wPianoMouseWheel,
    .terminate = 0
};

void wPianoInit(
    WidgetPiano* v,
    const char* name,
    uint8_t scale,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    SDL_assert(scale);
    widgetInit(&v->v, (void*)v, &wPianoApi, x, y, (KEY_WIDTH * WID_PIANO_NUM_KEYS + 1) * scale, (KEY_HEIGHT * 2) * scale + TOP_HEIGHT, panel.widget_scale, rend);
    v->name = name;
    v->scale = scale;
    v->velocity = 100;
    v->octave = 3;
    v->channel = 0;
    v->pointed_bmp = 0;
    v->press_mouse_bmp = 0;
    v->press_keyb_bmp = 0;
    v->press_int_bmp = 0;
    v->update_bmp = 0xFFFFFFFF;
}
