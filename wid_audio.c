#include "wid_audio.h"
#include "wid_graphics.h"
#include <portaudio.h>

#include "stdio.h"
#define WAUDIO_PRINTF printf
// #define WAUDIO_PRINTF(...)

__attribute__((unused)) static unsigned portaudio_requests = 0;
__attribute__((unused)) static PaError portaudio_status;

static void portaudioInitGlobal()
{
    if (0 == portaudio_requests) {
        portaudio_status = Pa_Initialize();
        WAUDIO_PRINTF("\nPa_Initialize: %s", Pa_GetErrorText(portaudio_status));
    }
    portaudio_requests++;
}

static void portaudioTerminateGlobal()
{
    portaudio_requests--;
    if (0 == portaudio_requests) {
        PaError err;
        err = Pa_Terminate();
        printf("\nPa_Terminate: %s", Pa_GetErrorText(err));
    }
}

// void audioBufferProcessCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length);

// __attribute__((weak)) void audioBufferProcessCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length)
// {
//     static float left_phase = 0;
//     static float right_phase = 0;
//     int16_t* __restrict in = buffer_in;
//     int16_t* __restrict out = buffer_out;
//     const int16_t* out_end = buffer_out + (length * 2); // stereo
//     // heavily inspired by the KORG Logue SDK https://github.com/korginc/logue-sdk
//     for (; out_end > out; out += 2, in += 2) {
//         // out[0] = in[0];
//         // out[1] = in[1];
//         out[0] = left_phase * 32767.f * 0.1f + in[0];
//         out[1] = right_phase * 32767.f * 0.1f + in[1];
//         // Generate simple -1.0 and 1.0 sawtooth
//         left_phase += 0.01001f;
//         if (left_phase >= 1.0f)
//             left_phase -= 2.0f;
//         right_phase += 0.012f;
//         if (right_phase >= 1.0f)
//             right_phase -= 2.0f;
//     }
// }

static void wAudioRedraw(void* wid)
{
    WidgetAudio* v = (WidgetAudio*)wid;
    SDL_FillRect(v->v.surface, 0, 0);
    drawOutline(&v->v, v->pointed ? panel.widget_color_pointed : panel.widget_color_released);
    if (v->pointed) {
        drawU16Centered(&v->v, 20, 2, v->blockcounter, panel.widget_color_pressed);
        drawU16Centered(&v->v, 20, 10, v->lastblocksize, panel.widget_color_pressed);
        v->v.need_redraw = 1; // ))
    } else {
        if ((!v->errorcounter) || (0 == v->error_last)) {
            uint32_t color = panel.widget_color_pressed;
            if (VAUDIO_ACTIVE == v->status) {
                color = panel.widget_color_released;
            }
            drawString(&v->v, 2, 2, v->name_in, color);
            drawString(&v->v, 2, 10, v->name_out, color);
        } else {
            drawU16Centered(&v->v, 20, 2, v->errorcounter_prev, panel.widget_color_pressed);
            drawU16Centered(&v->v, 20, 10, v->error_last, panel.widget_color_pressed);
        }
    }
}

static void wAudioProcess(void* wid, uint32_t clock)
{
    (void)clock;
    WidgetAudio* v = (WidgetAudio*)wid;
    const uint16_t errorcounter = v->errorcounter;
    if (errorcounter != v->errorcounter_prev) {
        v->errorcounter_prev = errorcounter;
        v->v.need_redraw = 1;
    }
}

static void wAudioTerminate(void* wid)
{
    WidgetAudio* v = (WidgetAudio*)wid;
    WAUDIO_PRINTF("\n wAudioTerminate");

    if (VAUDIO_OFF != v->status) {
        PaError err;
        if ((VAUDIO_ACTIVE == v->status) || (VAUDIO_ERROR == v->status)) {
            err = Pa_StopStream((PaStream*)v->instance);
            printf("\nPa_StopStream: %s", Pa_GetErrorText(err));
            // err = Pa_AbortStream( stream ); // immediately
            err = Pa_CloseStream((PaStream*)v->instance);
            printf("\nPa_CloseStream: %s", Pa_GetErrorText(err));
        }
        portaudioTerminateGlobal();
    }
}

static void wAudioMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements)
{
    WidgetAudio* v = (WidgetAudio*)wid;
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
static void wAudioMouseClick(void* wid, WidgetTouchData* d)
{
    WidgetAudio* v = (WidgetAudio*)wid;
    if (SDL_PointInRect(&d->point, &v->v.rect)) {
        WAUDIO_PRINTF("\n clean reset");
        v->error_last = 0;
    }
}
static WidgetApi wAudioApi = {
    .redraw = wAudioRedraw,
    .process = wAudioProcess,
    .keyboard = 0,
    .touchMove = wAudioMouseMove,
    .touchClick = wAudioMouseClick,
    .mouseWheel = 0,
    .terminate = wAudioTerminate
};

void wAudioInit(
    WidgetAudio* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t samplerate,
    uint32_t blocksize,
    PaStreamCallback* pa_callback_name)
{
    static int already_inited = 0;
    SDL_assert(0 == already_inited);
    already_inited++; // singletone

    if (dev_name_in)
        SDL_strlcpy(v->name_in, dev_name_in, VIDAUDIO_NAMELENGTH);
    else
        v->name_in[0] = 0;
    if (dev_name_out)
        SDL_strlcpy(v->name_out, dev_name_out, VIDAUDIO_NAMELENGTH);
    else
        v->name_out[0] = 0;
    v->samplerate = samplerate;
    v->blocksize = blocksize;

    widgetInit(&v->v, (void*)v, &wAudioApi, x, y, 40, 19, 1, rend);
    v->status = VAUDIO_OFF;
    v->instance = 0;
    v->errorcounter = v->errorcounter_prev = v->error_last = 0;
    v->blockcounter = 0;

    WAUDIO_PRINTF("\n wAudioInit");

    PaError err;
    portaudioInitGlobal(); // this will modify global portaudio_status
    if (paNoError == portaudio_status) {
        if ((0 == v->name_in[0]) && (0 == v->name_out[0])) {
            // names are not defined, use default devices
            err = Pa_OpenDefaultStream((PaStream**)&v->instance, 2, 2, paInt16, v->samplerate, v->blocksize, pa_callback_name, v);
            WAUDIO_PRINTF("\nPa_OpenDefaultStream: %s", Pa_GetErrorText(err));
            if (0 == err) {
                err = Pa_StartStream((PaStream*)v->instance);
                printf("\nPa_StartStream: %s", Pa_GetErrorText(err));
            }
            v->status = (0 == err) ? VAUDIO_ACTIVE : VAUDIO_ERROR;
            // get names
            const PaDeviceIndex din = Pa_GetDefaultInputDevice();
            const PaDeviceInfo* din_info = Pa_GetDeviceInfo(din);
            const PaDeviceIndex dout = Pa_GetDefaultOutputDevice();
            const PaDeviceInfo* dout_info = Pa_GetDeviceInfo(dout);
            SDL_strlcpy(v->name_in, din_info->name, VIDAUDIO_NAMELENGTH);
            SDL_strlcpy(v->name_out, dout_info->name, VIDAUDIO_NAMELENGTH);
            WAUDIO_PRINTF(
                "\n    in: %d, latency: %g, channels: %d, name: %s",
                din, din_info->defaultLowInputLatency, din_info->maxInputChannels, din_info->name);
            WAUDIO_PRINTF(
                "\n    out: %d, latency: %g, channels: %d, name: %s",
                dout, dout_info->defaultLowOutputLatency, dout_info->maxOutputChannels, dout_info->name);
        } else {
            // names were defined. let's find them in devices list
            v->status = VAUDIO_NOTFOUND;
            const PaDeviceIndex num_devices = Pa_GetDeviceCount();

            if (num_devices > 0) {
                PaStreamParameters stream_in;
                stream_in.device = -1;
                stream_in.channelCount = 2;
                stream_in.sampleFormat = paInt16;
                stream_in.suggestedLatency = 10;
                stream_in.hostApiSpecificStreamInfo = NULL;
                PaStreamParameters stream_out;
                stream_out.device = -1;
                stream_out.channelCount = 2;
                stream_out.sampleFormat = paInt16;
                stream_out.suggestedLatency = 10;
                stream_out.hostApiSpecificStreamInfo = NULL;
                // scan all
                for (int dev = 0; dev < num_devices; dev++) {
                    const PaDeviceInfo* info = Pa_GetDeviceInfo(dev);
                    if (info->maxInputChannels) {
                        WAUDIO_PRINTF(
                            "\n    in: %d, latency: %g, channels: %d, name: %s",
                            dev, info->defaultLowInputLatency, info->maxInputChannels, info->name);
                        if ((SDL_strstr(info->name, v->name_in))
                            && (info->maxInputChannels > 1)
                            && (info->defaultLowInputLatency < stream_in.suggestedLatency)) {
                            stream_in.device = dev;
                            stream_in.suggestedLatency = info->defaultLowInputLatency;
                        }
                    }
                    if (info->maxOutputChannels) {
                        WAUDIO_PRINTF(
                            "\n   out: %d, latency: %g, channels: %d, name: %s",
                            dev, info->defaultLowOutputLatency, info->maxOutputChannels, info->name);
                        if ((SDL_strstr(info->name, v->name_out))
                            && (info->maxOutputChannels > 1)
                            && (info->defaultLowOutputLatency < stream_out.suggestedLatency)) {
                            stream_out.device = dev;
                            stream_out.suggestedLatency = info->defaultLowOutputLatency;
                        }
                    }
                }
                // start
                if ((-1 == stream_in.device) || (-1 == stream_out.device)) {
                    v->status = VAUDIO_NOTFOUND;
                    WAUDIO_PRINTF("\n Not found");
                } else {
                    WAUDIO_PRINTF("\nFound in: %d, out: %d", stream_in.device, stream_out.device);
                    const PaDeviceInfo* din_info = Pa_GetDeviceInfo(stream_in.device);
                    const PaDeviceInfo* dout_info = Pa_GetDeviceInfo(stream_out.device);
                    SDL_strlcpy(v->name_in, din_info->name, VIDAUDIO_NAMELENGTH - 2);
                    SDL_strlcpy(v->name_out, dout_info->name, VIDAUDIO_NAMELENGTH - 2);
                    err = Pa_OpenStream((PaStream**)&v->instance, &stream_in, &stream_out, v->samplerate, v->blocksize, paNoFlag, pa_callback_name, v);
                    WAUDIO_PRINTF("\nPa_OpenStream: %s", Pa_GetErrorText(err));
                    if (0 == err) {
                        err = Pa_StartStream((PaStream*)v->instance);
                        printf("\nPa_StartStream: %s", Pa_GetErrorText(err));
                    }
                    v->status = (0 == err) ? VAUDIO_ACTIVE : VAUDIO_ERROR;
                }
            }
        }
    }
}
