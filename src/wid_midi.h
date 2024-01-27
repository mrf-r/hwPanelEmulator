#ifndef __WID_MIDI_H
#define __WID_MIDI_H

#include "wid_graphics.h"
#include "widget.h"
#include "portmidi.h"
#include "libserialport.h"
#include "midi_output.h"

/*
 version 1 - without any fancy menues, just a button to full reconnect
 settings: input name, output name
 this should work with both midi (portmidi) and com ports (libserialport)
 this module should not have direct connections to panel loop, only through mbwmidi
 direct interface is for debug only

 version2 - with select menu
*/

#define VIDMIDI_NAMELENGTH 32

typedef enum {
    VMIDI_OFF = 0,
    VMIDI_NOTFOUND,
    VMIDI_ERROR_PM,
    VMIDI_ERROR_SER,
    VMIDI_ACTIVE_PM,
    VMIDI_ACTIVE_SER,
};

typedef struct {
    Widget v;
    char name_in[VIDMIDI_NAMELENGTH];
    char name_out[VIDMIDI_NAMELENGTH];
    uint32_t baud;
    PortMidiStream* pm_input;
    PortMidiStream* pm_output;
    sp_port* ser_input;
    sp_port* ser_output;
    uint8_t status_in;
    uint8_t status_out;

} WidgetMidi;

void wMidiStart(WidgetMidi* v)
{
    // init midi and try to find needed ports
    PmError err;
    err = Pm_Initialize();
    int dev_count = Pm_CountDevices();

    v->status_in = VMIDI_NOTFOUND;
    v->status_out = VMIDI_NOTFOUND;

    const PmDeviceInfo* device_info;
    for (int dev = 0; dev < dev_count; dev++) {
        device_info = Pm_GetDeviceInfo(dev);
        if (device_info) {
            if ((VMIDI_NOTFOUND == v->status_in) && (device_info->input)) {
                if (SDL_strstr(device_info->name, v->name_in)) {
                    if (0 == Pm_OpenInput(&v->pm_input, dev, NULL, 256, NULL, NULL)) {
                        v->status_in = VMIDI_ACTIVE_PM;
                    } else {
                        v->status_in = VMIDI_ERROR_PM;
                        v->pm_input = 0;
                    }
                }
            } else if ((VMIDI_NOTFOUND == v->status_out) && (device_info->output)) {
                if (SDL_strstr(device_info->name, v->name_in)) {
                    if (0 == Pm_OpenOutput(&v->pm_output, dev, NULL, 256, NULL, NULL, 0)) {
                        v->status_out = VMIDI_ACTIVE_PM;
                    } else {
                        v->status_out = VMIDI_ERROR_PM;
                        v->pm_output = 0;
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
    // printf("Getting port list.\n");
    if (SP_OK == sp_list_ports(&port_list)) {
        for (int i = 0; port_list[i] != NULL; i++) {
            struct sp_port* port = port_list[i];
            // port description is not great for silabs/ch340 adapters as they usually unnamed
            // and we need COMxx crappy name anyway
            char* port_desc = sp_get_port_description(port);
            if ((VMIDI_NOTFOUND == v->status_in) && (SDL_strstr(port_desc, v->name_in))) {
                if (SP_OK != sp_copy_port(port, &v->ser_input))
                    goto ser_in_error;
                if (SP_OK != sp_open(v->ser_input, SP_MODE_READ_WRITE))
                    goto ser_in_error;
                if (SP_OK != sp_set_config(v->ser_input, config))
                    goto ser_in_error;
                v->status_in = VMIDI_ACTIVE_SER;
                if (0) {
                ser_in_error:
                    v->ser_input = 0;
                    v->status_in = VMIDI_ERROR_SER;
                }
            }
            if ((VMIDI_NOTFOUND == v->status_out) && (SDL_strstr(port_desc, v->name_out))) {
                char* name_this = sp_get_port_name(port);
                char* name_in = "==undefined==";
                if (v->ser_input)
                    name_in = sp_get_port_name(v->ser_input);
                if (0 == SDL_strcmp(name_in, name_this)) {
                    // same port
                    v->ser_output = v->ser_input;
                    v->status_out = VMIDI_ACTIVE_SER;
                } else {
                    // open another one for output
                    if (SP_OK != sp_copy_port(port, &v->ser_output))
                        goto ser_in_error;
                    if (SP_OK != sp_open(v->ser_output, SP_MODE_READ_WRITE))
                        goto ser_in_error;
                    if (SP_OK != sp_set_config(v->ser_output, config))
                        goto ser_in_error;
                    v->status_out = VMIDI_ACTIVE_SER;
                    if (0) {
                    ser_in_error:
                        v->ser_output = 0;
                        v->status_out = VMIDI_ERROR_SER;
                    }
                }
            }
        }
    }
    sp_free_config(config);
    sp_free_port_list(port_list);
}

void wMidiStop(WidgetMidi* v)
{
    if (v->pm_input) {
        Pm_Close(v->pm_input);
        v->pm_input = 0;
        v->status_in = VMIDI_OFF;
    }
    if (v->pm_output) {
        Pm_Close(v->pm_output);
        v->pm_output = 0;
        v->status_out = VMIDI_OFF;
    }
    Pm_Terminate();

    if (v->ser_input) {
        sp_close(v->ser_input);
        sp_free_port(v->ser_input);
        v->ser_input = 0;
        v->status_in = VMIDI_OFF;
    }
    if (v->ser_output) {
        sp_close(v->ser_output);
        sp_free_port(v->ser_output);
        v->ser_output = 0;
        v->status_out = VMIDI_OFF;
    }
}

static void wMidiRedraw(void* wid)
{
    WidgetMidi* v = (WidgetMidi*)wid;
    if (v->v.need_redraw) {
        v->v.need_redraw = 0;
        drawOutline(&v->v, widget_color_released);
    }
}

static WidgetApi wMidiApi = {
    .redraw = 0,
    .process = 0,
    .keyboard = 0,
    .mouseMove = 0, // highlight
    .mouseClick = 0, // click to menu switch
    .mouseWheel = 0
};

static void wFrameCounterInit(
    WidgetMidi* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend)
{
    widgetInit(&v->v, (void*)v, &wMidiApi, x, y, 40, 16, 1, rend);

    // we need to read needed midi devices from configuration file
    // if there are no file or midi devices, then create file or append these parameters to existing
}

void wMidiLoadConfig()
{
    size_t filesize;
    // void *file = SDL_LoadFile("1", &filesize);
    // SDL_sscanf
    // SDL_strstr
    // SDL_snprintf
    // SDL_sscanf
    //
}

void wMidiSaveConfig()
{
    //
}

#endif // __WID_MIDI_H