#include "wid_midi.h"
#include "wid_graphics.h"
#include "mbwmidi.h"

#include "stdio.h"
// #define WMIDI_PRINTF printf
#define WMIDI_PRINTF(...)

#include "midi_portmidi.h"
#include "midi_libserialport.h"

MidiOutPortContextT midi_out_port; // nonstatic

static void wMidiStart(WidgetMidi* v)
{
    v->status_in = VMIDI_NOTFOUND;
    v->status_out = VMIDI_NOTFOUND;

    wmpmStart(v);
    wmlsStart(v);
    WMIDI_PRINTF("\n wMidiStart");
}

static void wMidiStop(WidgetMidi* v)
{
    wmpmStop(v);
    wmlsStop(v);
    WMIDI_PRINTF("\n wMidiStop");
}

static void wMidiRedraw(void* wid)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    SDL_FillRect(v->v.surface, 0, 0);
    drawOutline(&v->v, v->pointed ? panel.widget_color_pointed : panel.widget_color_released);
    if (v->pointed) {
        v->name_in[VIDMIDI_NAMELENGTH - 1] = 0;
        v->name_out[VIDMIDI_NAMELENGTH - 1] = 0;
        uint32_t color = panel.widget_color_pressed;
        if ((VMIDI_ACTIVE_PM == v->status_in) || (VMIDI_ACTIVE_SER == v->status_in)) {
            color = panel.widget_color_released;
        }
        drawString(&v->v, 2, 2, v->name_in, color);
        color = panel.widget_color_pressed;
        if ((VMIDI_ACTIVE_PM == v->status_out) || (VMIDI_ACTIVE_SER == v->status_out)) {
            color = panel.widget_color_released;
        }
        drawString(&v->v, 2, 10, v->name_out, color);
    } else {
        drawU16Centered(&v->v, 20, 2, v->counter_in, panel.widget_color_helptext);
        drawU16Centered(&v->v, 20, 10, v->counter_out, panel.widget_color_helptext);
    }
}

static void wMidiProcess(void* wid, uint32_t clock)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (VMIDI_ACTIVE_PM == v->status_in) {
        wmpmProcessIn(v, clock);
    } else if (VMIDI_ACTIVE_SER == v->status_in) {
        wmlsProcessIn(v, clock);
    }
    // limit the output flow
    // 1 - calculate time
    const uint32_t time_delta_clk = clock - v->proc_clk_prev;
    v->proc_clk_prev = clock;
    const uint32_t time_delta_us = time_delta_clk * 1000000 / MIDI_CLOCK_RATE + v->proc_prev_remaining_us;
    const uint32_t bytes_per_second = v->baud_virtual / 10;
    // const uint32_t messages_per_second = bytes_per_second * 5 / 2; // ~2.5 bytes per message
    const uint32_t byte_time_us = 1000000 / bytes_per_second;
    const uint32_t bytes_in_this_timeslot = time_delta_us / byte_time_us;
    v->proc_prev_remaining_us = time_delta_us - bytes_in_this_timeslot * byte_time_us;

    if (VMIDI_ACTIVE_PM == v->status_out) {
        wmpmProcessOut(v, clock, bytes_in_this_timeslot);
    } else if (VMIDI_ACTIVE_SER == v->status_out) {
        wmlsProcessOut(v, clock, bytes_in_this_timeslot);
    }
    if ((v->counter_in_prev != v->counter_in) || (v->counter_out_prev != v->counter_out)) {
        v->counter_in_prev = v->counter_in;
        v->counter_out_prev = v->counter_out;
        v->v.need_redraw = 1;
    }
}
static void wMidiMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetMidi* v = (WidgetMidi*)wid;
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
static void wMidiMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        WMIDI_PRINTF("\n midi reset");
        wMidiStop(v);
        // do something funny while midi is off?
        wMidiStart(v);
        v->v.need_redraw = 1;
    }
}
static void wMidiTerminate(void* wid)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    WMIDI_PRINTF("\n wMidiTerminate");
    wMidiStop(v);
}

static WidgetApi wMidiApi = {
    .redraw = wMidiRedraw,
    .process = wMidiProcess,
    .keyboard = 0,
    .touchMove = wMidiMouseMove, // highlight
    .touchClick = wMidiMouseClick, // click to menu switch
    .mouseWheel = 0,
    .terminate = wMidiTerminate
};

void wMidiInit(
    WidgetMidi* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t baud_virtual,
    uint32_t baud_serial)
{
    static int already_inited = 0;
    SDL_assert(0 == already_inited);
    already_inited++; // singletone

    SDL_strlcpy(v->name_in, dev_name_in, VIDMIDI_NAMELENGTH);
    SDL_strlcpy(v->name_out, dev_name_out, VIDMIDI_NAMELENGTH);
    v->baud_virtual = baud_virtual;
    v->baud_physical_serial = baud_serial;

    widgetInit(&v->v, (void*)v, &wMidiApi, x, y, 40, 19, 1, rend);
    v->status_in = v->status_out = VMIDI_OFF;
    v->inst_input = v->inst_output = 0;
    v->counter_in = v->counter_in_prev = 0;
    v->counter_out = v->counter_out_prev = 0;
    v->pm_sysex_in_len = v->pm_sysex_out_len = 0;
    if (v->baud_virtual < 10)
        v->baud_virtual = 10;

    WMIDI_PRINTF("\n midi start");
    wMidiStart(v);

    midiInit();
    midiPortInit(&midi_out_port);

    // we need to read needed midi devices from configuration file
    // if there are no file or midi devices, then create file or append these parameters to existing
}

// void wMidiLoadConfig()
// {
//     size_t filesize;
//     // void *file = SDL_LoadFile("1", &filesize);
//     // SDL_sscanf
//     // SDL_strstr
//     // SDL_snprintf
//     // SDL_sscanf
//     //
// }

// void wMidiSaveConfig()
// {
//     //
// }
