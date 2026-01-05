#ifndef _MIDI_LIBSERIALPORT_H
#define _MIDI_LIBSERIALPORT_H

#include "wid_midi.h"
#include <libserialport.h>
#include "mbwmidi.h"

#ifndef WMIDI_PRINTF
#define WMIDI_PRINTF(...)
#endif

static MidiOutUartContextT midi_uart_out_cx;
static MidiInUartContextT midi_uart_in_cx;
static unsigned out_is_busy;
static struct sp_port* out_port;

static void uartSendByte(const uint8_t b)
{
    out_is_busy = 1;
    int result = sp_nonblocking_write(out_port, &b, 1);
    if (1 != result) {
        WMIDI_PRINTF("\n uartSendByte failed with error: %d", result);
    }
}
static void uartStopSend()
{
    out_is_busy = 0;
}
static MidiRet uartIsBusy()
{
    return out_is_busy ? MIDI_RET_OK : MIDI_RET_FAIL;
}

static MidiOutUartPortT midi_uart_out = {
    .port = &midi_out_port,
    .context = &midi_uart_out_cx,
    .sendByte = uartSendByte,
    .stopSend = uartStopSend,
    .isBusy = uartIsBusy
};

static void wmlsStart(WidgetMidi* v)
{
    struct sp_port** port_list;
    struct sp_port_config* config;
    sp_new_config(&config);
    sp_set_config_baudrate(config, v->baud_physical_serial);
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
                    if (SP_OK != sp_copy_port(port, (struct sp_port**)&v->inst_output))
                        goto ser_out_error;
                    if (SP_OK != sp_open((struct sp_port*)v->inst_output, SP_MODE_READ_WRITE))
                        goto ser_out_error;
                    if (SP_OK != sp_set_config((struct sp_port*)v->inst_output, config))
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

    if (VMIDI_ACTIVE_SER == v->status_in) {
        WMIDI_PRINTF("\n pm in active");
        midiInUartInit(&midi_uart_in_cx);
    }
    if (VMIDI_ACTIVE_SER == v->status_out) {
        WMIDI_PRINTF("\n pm out active");
        midiOutUartInit(&midi_uart_out);
        out_is_busy = 0;
        out_port = (struct sp_port*)v->inst_output;
    }
}

static void wmlsStop(WidgetMidi* v)
{
    if (VMIDI_ACTIVE_SER == v->status_in) {
        sp_close((struct sp_port*)v->inst_input);
        sp_free_port((struct sp_port*)v->inst_input);
        v->inst_input = 0;
        v->status_in = VMIDI_OFF;
    }
    if (VMIDI_ACTIVE_SER == v->status_out) {
        sp_close((struct sp_port*)v->inst_output);
        sp_free_port((struct sp_port*)v->inst_output);
        v->inst_output = 0;
        v->status_out = VMIDI_OFF;
    }
}

#define WMLS_READ_BUFFER_SIZE 16 // doesn't have much effect

static void wmlsProcessIn(WidgetMidi* v, uint32_t clock)
{
    SDL_assert(VMIDI_ACTIVE_SER == v->status_in);
    (void)clock;
    midiInUartTap(&midi_uart_in_cx, MIDI_CN_USB_DEVICE);
    uint8_t read_buffer[WMLS_READ_BUFFER_SIZE];
    int result;
    do {
        result = sp_nonblocking_read((struct sp_port*)v->inst_input, read_buffer, WMLS_READ_BUFFER_SIZE);
        for (int i = 0; i < result; i++) {
            midiInUartByteReceiveCallback(read_buffer[i], &midi_uart_in_cx, MIDI_CN_USB_DEVICE);
        }
    } while (WMLS_READ_BUFFER_SIZE == result);
}

static void wmlsProcessOut(WidgetMidi* v, const uint32_t clock, const uint32_t bytes_in_this_timeslot)
{
    SDL_assert(VMIDI_ACTIVE_SER == v->status_out);
    (void)clock;
    midiOutUartTap(&midi_uart_out);
    // uart interrupt emulation
    for (unsigned i = 0; i < bytes_in_this_timeslot; i++) {
        if (MIDI_RET_OK == uartIsBusy()) {
            midiOutUartTranmissionCompleteCallback(&midi_uart_out);
        } else
            break;
    }
}

#endif // _MIDI_LIBSERIALPORT_H