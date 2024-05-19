#ifndef __WID_MIDI_H
#define __WID_MIDI_H

#include "widget.h"
#include "mbwmidi.h"

/*
 version 1 - without any fancy menues, just a button to full reconnect
 settings: input name, output name
 this should work with both midi (portmidi) and com ports (libserialport)
 this module should not have direct connections to panel loop, only through mbwmidi
 direct interface is for debug only

 version2 - with select menu ??
*/

#define VIDMIDI_NAMELENGTH 32

typedef enum {
    VMIDI_OFF = 0,
    VMIDI_NOTFOUND,
    VMIDI_ERROR_PM,
    VMIDI_ERROR_SER,
    VMIDI_ACTIVE_PM,
    VMIDI_ACTIVE_SER,
} WidMidiStatus;

typedef struct {
    Widget v;
    uint8_t pointed;
    char name_in[VIDMIDI_NAMELENGTH];
    char name_out[VIDMIDI_NAMELENGTH];
    uint32_t baud_virtual;
    uint32_t baud_physical_serial;
    // PortMidiStream* pm_input;
    // PortMidiStream* pm_output;
    // sp_port* ser_input;
    // sp_port* ser_output;
    void* inst_input;
    void* inst_output;
    uint16_t counter_in;
    uint16_t counter_out;
    uint16_t counter_in_prev;
    uint16_t counter_out_prev;
    uint8_t status_in;
    uint8_t status_out;
    uint8_t pm_sysex_in[4];
    uint8_t pm_sysex_in_len;
    uint8_t pm_sysex_out[4];
    uint8_t pm_sysex_out_len;
    uint32_t proc_clk_prev;
    uint32_t proc_prev_remaining_us;
} WidgetMidi;

void wMidiInit(
    WidgetMidi* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t baud_virtual,
    uint32_t baud_serial);

extern MidiOutPortContextT midi_out_port;

#endif // __WID_MIDI_H