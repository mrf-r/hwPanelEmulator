#include "wid_midi.h"
#include "wid_graphics.h"
#include "portmidi.h"
#include "libserialport.h"
#include "mbwmidi.h"

#include "stdio.h"

#define WMIDI_PRINTF printf
// #define WMIDI_PRINTF(...)

void wMidiStart(WidgetMidi* v)
{
    // init midi and try to find needed ports
    PmError err;
    err = Pm_Initialize();
    int dev_count = Pm_CountDevices();
    // WMIDI_PRINTF("\n default in: %d", Pm_GetDefaultInputDeviceID());
    // WMIDI_PRINTF("\n default out: %d", Pm_GetDefaultOutputDeviceID());

    v->status_in = VMIDI_NOTFOUND;
    v->status_out = VMIDI_NOTFOUND;

    const PmDeviceInfo* device_info;
    for (int dev = 0; dev < dev_count; dev++) {
        device_info = Pm_GetDeviceInfo(dev);
        if (device_info) {

            WMIDI_PRINTF("\n midi: %d - %s", dev, device_info->name);

            if ((VMIDI_NOTFOUND == v->status_in) && (device_info->input)) {
                if (SDL_strstr(device_info->name, v->name_in)) {
                    if (0 == Pm_OpenInput((PortMidiStream**)&v->inst_input, dev, NULL, 256, NULL, NULL)) {
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
                    if (0 == Pm_OpenOutput((PortMidiStream**)&v->inst_output, dev, NULL, 256, NULL, NULL, 0)) {
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
    } // devices count scan
    ////////////////////////////////////////////////////////////////////////////////////////////
    // init serial and try to find ports
    struct sp_port** port_list;
    struct sp_port_config* config;
    sp_new_config(&config);
    sp_set_config_baudrate(config, v->baud);
    sp_set_config_bits(config, 8);
    sp_set_config_parity(config, SP_PARITY_NONE);
    sp_set_config_stopbits(config, 1);
    sp_set_config_flowcontrol(config, SP_FLOWCONTROL_NONE);
    // WMIDI_PRINTF("Getting port list.\n");
    if (SP_OK == sp_list_ports(&port_list)) {
        for (int i = 0; port_list[i] != NULL; i++) {
            struct sp_port* port = port_list[i];
            // port description is not great for silabs/ch340 adapters as they usually unnamed
            // and we need COMxx crappy name anyway

            char* port_desc = sp_get_port_description(port);
            //
            WMIDI_PRINTF("\n ser: %s", port_desc);
            //

            if ((VMIDI_NOTFOUND == v->status_in) && (SDL_strstr(port_desc, v->name_in))) {
                if (SP_OK != sp_copy_port(port, (struct sp_port**)&v->inst_input))
                    goto ser_in_error;
                if (SP_OK != sp_open((struct sp_port*)v->inst_input, SP_MODE_READ_WRITE))
                    goto ser_in_error;
                if (SP_OK != sp_set_config((struct sp_port*)v->inst_input, config))
                    goto ser_in_error;
                v->status_in = VMIDI_ACTIVE_SER;
                SDL_strlcpy(v->name_in, port_desc, VIDMIDI_NAMELENGTH - 2);
                if (0) {
                ser_in_error:
                    v->inst_input = 0;
                    v->status_in = VMIDI_ERROR_SER;
                }
            }
            if ((VMIDI_NOTFOUND == v->status_out) && (SDL_strstr(port_desc, v->name_out))) {
                char* name_this = sp_get_port_name(port);
                char* name_in = "==undefined==";
                if (v->inst_input)
                    name_in = sp_get_port_name((struct sp_port*)v->inst_input);
                if (0 == SDL_strcmp(name_in, name_this)) {
                    // same port
                    v->inst_output = v->inst_input;
                    v->status_out = VMIDI_ACTIVE_SER;
                    SDL_strlcpy(v->name_out, port_desc, VIDMIDI_NAMELENGTH - 2);
                } else {
                    // open another one for output
                    if (SP_OK != sp_copy_port(port, &v->inst_output))
                        goto ser_out_error;
                    if (SP_OK != sp_open(v->inst_output, SP_MODE_READ_WRITE))
                        goto ser_out_error;
                    if (SP_OK != sp_set_config(v->inst_output, config))
                        goto ser_out_error;
                    v->status_out = VMIDI_ACTIVE_SER;
                    SDL_strlcpy(v->name_out, port_desc, VIDMIDI_NAMELENGTH - 2);
                    if (0) {
                    ser_out_error:
                        v->inst_output = 0;
                        v->status_out = VMIDI_ERROR_SER;
                    }
                }
            }
        }
    }
    sp_free_config(config);
    sp_free_port_list(port_list);

    WMIDI_PRINTF("\n wMidiStart");
}

void wMidiStop(WidgetMidi* v)
{
    if (VMIDI_ACTIVE_PM == v->status_in) {
        Pm_Close((PortMidiStream*)v->inst_input);
        v->inst_input = 0;
        v->status_in = VMIDI_OFF;
    } else if (VMIDI_ACTIVE_SER == v->status_in) {
        sp_close((struct sp_port*)v->inst_input);
        sp_free_port((struct sp_port*)v->inst_input);
        v->inst_input = 0;
        v->status_in = VMIDI_OFF;
    }

    if (VMIDI_ACTIVE_PM == v->status_out) {
        Pm_Close((PortMidiStream*)v->inst_output);
        v->inst_output = 0;
        v->status_out = VMIDI_OFF;
    } else if (VMIDI_ACTIVE_SER == v->status_out) {
        sp_close(v->inst_output);
        sp_free_port(v->inst_output);
        v->inst_output = 0;
        v->status_out = VMIDI_OFF;
    }

    Pm_Terminate();
    WMIDI_PRINTF("\n wMidiStop");
}

static void wMidiRedraw(void* wid)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
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
}

static inline void pmEventSysexReceive(WidgetMidi* v, PmEvent ev)
{
    // sysex timestamps are ignored
    for (int i = 0; i < 4; i++) {
        uint8_t b = (ev.message >> (i * 4)) & 0xFF;
        if (b = 0xF7) {
            // termination
            MidiMessageT m;
            m.cn = MIDI_CN_USB_DEVICE;
            switch (v->pm_sysex_len) {
            case 0:
                m.cin = MIDI_CIN_SYSEXEND1;
                m.byte1 = b;
                m.byte2 = v->pm_sysex[1];
                m.byte3 = v->pm_sysex[2];
                break;
            case 1:
                m.cin = MIDI_CIN_SYSEXEND2;
                m.byte1 = v->pm_sysex[0];
                m.byte2 = b;
                m.byte3 = v->pm_sysex[2];
                break;
            case 2:
                m.cin = MIDI_CIN_SYSEXEND3;
                m.byte1 = v->pm_sysex[0];
                m.byte2 = v->pm_sysex[1];
                m.byte3 = b;
                break;
            }
            midiTsWrite(m, 0);
            v->pm_sysex_len = 0;
        } else {
            v->pm_sysex[v->pm_sysex_len] = b;
            v->pm_sysex_len++;
            if (v->pm_sysex_len == 3) {
                MidiMessageT m;
                m.cn = MIDI_CN_USB_DEVICE;
                m.cin = MIDI_CIN_SYSEX3BYTES;
                m.byte1 = v->pm_sysex[0];
                m.byte2 = v->pm_sysex[1];
                m.byte3 = v->pm_sysex[2];
                midiTsWrite(m, 0);
                v->pm_sysex_len = 0;
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

static void wMidiProcess(void* wid, uint32_t clock)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (VMIDI_ACTIVE_PM == v->status_in) {
        PmError err;
        PmEvent rx;
        while (1 == Pm_Read((PortMidiStream*)v->inst_input, &rx, 1)) {
            pmEventReceive(v, rx);
        }
    } else if (VMIDI_ACTIVE_SER == v->status_in) {
        // ser
    }
    // output
    if (VMIDI_ACTIVE_PM == v->status_out) {
        //
    } else if (VMIDI_ACTIVE_SER == v->status_out) {
        // ser
    }
}
static void wMidiMouseMove(void* wid, SDL_Point* pos, uint8_t click)
{
    WidgetMidi* v = (WidgetMidi*)wid;
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
static void wMidiMouseClick(void* wid, SDL_Point* pos, Drag* d)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (SDL_PointInRect(pos, &v->v.rect)) {
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
    WMIDI_PRINTF("\n midi end");
    wMidiStop(v);
}

static WidgetApi wMidiApi = {
    .redraw = wMidiRedraw,
    .process = wMidiProcess,
    .keyboard = 0,
    .mouseMove = wMidiMouseMove, // highlight
    .mouseClick = wMidiMouseClick, // click to menu switch
    .mouseWheel = 0,
    .terminate = wMidiTerminate
};

void wMidiInit(
    WidgetMidi* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wMidiApi, x, y, 40, 19, 1, rend);
    v->status_in = v->status_out = VMIDI_OFF;
    v->inst_input = v->inst_output = 0;
    v->counter_in = v->counter_in_prev = 0;
    v->counter_out = v->counter_out_prev = 0;

    WMIDI_PRINTF("\n midi start");
    wMidiStart(v);

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

/*
TODO:
- fix pointers!!!
-pm
    - receiver
    - unify timestamps ??
    - transmitter
    - baud based output limiter (?)
-ser:
    - baud based output limiter (use process clock)
    - tap
*/

// MidiOutPortContextT orca_uart_port;
// static MidiOutUartContextT orca_uart_cx;

// static void ouSendByte(uint8_t b)
// {
//     UART0->THR = b;
//     UART0->IER = UART_IER_THRE_IEN_Msk;
// }
// static void ouStopSend()
// {
//     UART0->IER = 0;
// }
// static MidiRet ouIsBusy()
// {
//     return (UART0->IER & UART_IER_THRE_IEN_Msk) ? MIDI_RET_OK : MIDI_RET_FAIL;
// }

// MidiOutUartPortT orca_uart = {
//     .port = &orca_uart_port,
//     .context = &orca_uart_cx,
//     .sendByte = ouSendByte,
//     .stopSend = ouStopSend,
//     .isBusy = ouIsBusy
// };
