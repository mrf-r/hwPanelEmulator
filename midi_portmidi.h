#ifndef _MIDI_PORTMIDI_H
#define _MIDI_PORTMIDI_H

#include "mbwmidi.h"
#include "wid_midi.h"
#include <SDL.h>
#include <portmidi.h>

#ifndef WMIDI_PRINTF
#define WMIDI_PRINTF(...)
#endif

static int wmpmTime(void* time_info)
{
    (void)time_info;
    return SDL_GetTicks();
}

static void wmpmStart(WidgetMidi* v)
{
    // init midi and try to find needed ports
    // PmError err;
    Pm_Initialize();
    int dev_count = Pm_CountDevices();
    // WMIDI_PRINTF("\n default in: %d", Pm_GetDefaultInputDeviceID());
    // WMIDI_PRINTF("\n default out: %d", Pm_GetDefaultOutputDeviceID());

    const PmDeviceInfo* device_info;
    for (int dev = 0; dev < dev_count; dev++) {
        device_info = Pm_GetDeviceInfo(dev);
        if (device_info) {

            WMIDI_PRINTF("\n midi: %d - %s", dev, device_info->name);

            if ((VMIDI_NOTFOUND == v->status_in) && (device_info->input)) {
                if (SDL_strstr(device_info->name, v->name_in)) {
                    if (0 == Pm_OpenInput((PortMidiStream**)&v->inst_input, dev, NULL, 256, wmpmTime, NULL)) {
                        v->status_in = VMIDI_ACTIVE_PM;
                        SDL_strlcpy(v->name_in, device_info->name, VIDMIDI_NAMELENGTH - 2);
                        WMIDI_PRINTF("\n opened in: %d - %s", dev, device_info->name);
                    } else {
                        v->status_in = VMIDI_ERROR_PM;
                        v->inst_input = 0;
                    }
                }
            } else if ((VMIDI_NOTFOUND == v->status_out) && (device_info->output)) {
                if (SDL_strstr(device_info->name, v->name_out)) {
                    if (0 == Pm_OpenOutput((PortMidiStream**)&v->inst_output, dev, NULL, 256, wmpmTime, NULL, 0)) {
                        v->status_out = VMIDI_ACTIVE_PM;
                        SDL_strlcpy(v->name_out, device_info->name, VIDMIDI_NAMELENGTH - 2);
                        WMIDI_PRINTF("\n opened out: %d - %s", dev, device_info->name);
                    } else {
                        v->status_out = VMIDI_ERROR_PM;
                        v->inst_output = 0;
                    }
                }
            }
        }
    }
}

static void wmpmStop(WidgetMidi* v)
{
    if (VMIDI_ACTIVE_PM == v->status_in) {
        Pm_Close((PortMidiStream*)v->inst_input);
        v->inst_input = 0;
        v->status_in = VMIDI_OFF;
    }
    if (VMIDI_ACTIVE_PM == v->status_out) {
        Pm_Close((PortMidiStream*)v->inst_output);
        v->inst_output = 0;
        v->status_out = VMIDI_OFF;
    }
    Pm_Terminate();
}

static inline void pmEventSysexReceive(WidgetMidi* v, PmEvent ev)
{
    // sysex timestamps are ignored
    for (int i = 0; i < 4; i++) {
        uint8_t b = (ev.message >> (i * 4)) & 0xFF;
        if (0xF7 == b) {
            // termination
            MidiMessageT m;
            m.cn = MIDI_CN_USB_DEVICE;
            switch (v->pm_sysex_in_len) {
            case 0:
                m.cin = MIDI_CIN_SYSEXEND1;
                m.byte1 = b;
                m.byte2 = v->pm_sysex_in[1];
                m.byte3 = v->pm_sysex_in[2];
                break;
            case 1:
                m.cin = MIDI_CIN_SYSEXEND2;
                m.byte1 = v->pm_sysex_in[0];
                m.byte2 = b;
                m.byte3 = v->pm_sysex_in[2];
                break;
            case 2:
                m.cin = MIDI_CIN_SYSEXEND3;
                m.byte1 = v->pm_sysex_in[0];
                m.byte2 = v->pm_sysex_in[1];
                m.byte3 = b;
                break;
            }
            midiTsWrite(m, 0);
            v->pm_sysex_in_len = 0;
        } else {
            v->pm_sysex_in[v->pm_sysex_in_len] = b;
            v->pm_sysex_in_len++;
            if (v->pm_sysex_in_len == 3) {
                MidiMessageT m;
                m.cn = MIDI_CN_USB_DEVICE;
                m.cin = MIDI_CIN_SYSEX3BYTES;
                m.byte1 = v->pm_sysex_in[0];
                m.byte2 = v->pm_sysex_in[1];
                m.byte3 = v->pm_sysex_in[2];
                midiTsWrite(m, 0);
                v->pm_sysex_in_len = 0;
            }
        }
    }
}

static inline void pmEventReceive(WidgetMidi* v, PmEvent ev)
{
    // TODO: realtime sync can be inserted in sysex. is that critical?
    // TODO: 2 and 3 bytes system common messages currently will be handles as single
    uint8_t status = Pm_MessageStatus(ev.message);
    uint32_t ts = ev.timestamp;
    if (status & 0x80) {
        if ((0xF1 == status) || (0xF3 == status)) {
            MidiMessageT m;
            m.full_word = ev.message << 8;
            m.cn = MIDI_CN_USB_DEVICE;
            m.cin = MIDI_CIN_2BYTESYSTEMCOMMON;
            midiTsWrite(m, ts);
        } else if (0xF2 == status) {
            MidiMessageT m;
            m.full_word = ev.message << 8;
            m.cn = MIDI_CN_USB_DEVICE;
            m.cin = MIDI_CIN_3BYTESYSTEMCOMMON;
            midiTsWrite(m, ts);
        } else if ((0xF0 == status) || (0xF7 == status)) {
            pmEventSysexReceive(v, ev);
        } else {
            MidiMessageT m;
            m.full_word = ev.message << 8;
            m.cn = MIDI_CN_USB_DEVICE;
            m.cin = status >> 4;
            midiTsWrite(m, ts);
        }
    } else {
        pmEventSysexReceive(v, ev);
    }
}

static inline void pmSyxOutCompose(WidgetMidi* v, uint8_t byte)
{
    v->pm_sysex_out[v->pm_sysex_out_len] = byte;
    v->pm_sysex_out_len++;
    if (4 == v->pm_sysex_out_len) {
        v->pm_sysex_out_len = 0;
        PmEvent ev = {
            .timestamp = 0,
            .message = v->pm_sysex_out[0]
                | (v->pm_sysex_out[1] << 8)
                | (v->pm_sysex_out[2] << 16)
                | (v->pm_sysex_out[3] << 24)
        };
        Pm_Write((PortMidiStream*)v->inst_output, &ev, 1);
        WMIDI_PRINTF("\n Pm_Write: %08X", ev.message);
    }
}

static void wmpmProcessIn(WidgetMidi* v, uint32_t clock)
{
    SDL_assert(VMIDI_ACTIVE_PM == v->status_in);

    PmEvent rx;
    // read all received messages
    while (1 == Pm_Read((PortMidiStream*)v->inst_input, &rx, 1)) {
        pmEventReceive(v, rx);
        v->counter_in++;
    }
}

static void wmpmProcessOut(WidgetMidi* v, const uint32_t clock, const uint32_t bytes_in_this_timeslot)
{
    SDL_assert(VMIDI_ACTIVE_PM == v->status_in);
    MidiMessageT m;
    uint32_t bytes_limit = bytes_in_this_timeslot;
    while ((bytes_limit) && (MIDI_RET_OK == midiPortReadNext(&midi_out_port, &m))) {
        v->counter_out++;
        switch (m.cin) {
        default:
            break;
        case MIDI_CIN_SYSEX3BYTES:
        case MIDI_CIN_SYSEXEND3:
            pmSyxOutCompose(v, m.byte1);
            if (bytes_limit)
                bytes_limit--;
            pmSyxOutCompose(v, m.byte2);
            if (bytes_limit)
                bytes_limit--;
            pmSyxOutCompose(v, m.byte3);
            if (bytes_limit)
                bytes_limit--;
            break;
        case MIDI_CIN_SYSEXEND2:
            pmSyxOutCompose(v, m.byte1);
            if (bytes_limit)
                bytes_limit--;
            pmSyxOutCompose(v, m.byte2);
            if (bytes_limit)
                bytes_limit--;
            break;
        case MIDI_CIN_SYSEXEND1:
            pmSyxOutCompose(v, m.byte1);
            if (bytes_limit)
                bytes_limit--;
            break;
        case MIDI_CIN_3BYTESYSTEMCOMMON:
        case MIDI_CIN_NOTEOFF:
        case MIDI_CIN_NOTEON:
        case MIDI_CIN_POLYKEYPRESS:
        case MIDI_CIN_CONTROLCHANGE:
        case MIDI_CIN_PITCHBEND:
            if (bytes_limit)
                bytes_limit--;
            // fall through
        case MIDI_CIN_2BYTESYSTEMCOMMON:
        case MIDI_CIN_PROGRAMCHANGE:
        case MIDI_CIN_CHANNELPRESSURE:
            if (bytes_limit)
                bytes_limit--;
            // fall through
        case MIDI_CIN_SINGLEBYTE:
            if (bytes_limit)
                bytes_limit--;
            {
                // const PmMessage msg = m.full_word >> 8;
                // Pm_WriteShort((PortMidiStream*)v->inst_output, 0, msg);
                PmEvent ev = {
                    .timestamp = 0,
                    .message = m.full_word >> 8
                };
                Pm_Write((PortMidiStream*)v->inst_output, &ev, 1);
                WMIDI_PRINTF("\n Pm_Write: %08X", ev.message);
            }
            break;
        }
    } // while port messages are present
}

#endif // _MIDI_PORTMIDI_H