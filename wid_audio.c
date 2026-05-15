#include "wid_audio.h"
#include "wid_graphics.h"
#include <portaudio.h>
#include <string.h>

#include <stdio.h>
#define WAUDIO_PRINTF printf
// #define WAUDIO_PRINTF(...)

static int pa_callback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    (void)timeInfo; // TODO: do something with timeInfo?
    WidgetAudio* v = (WidgetAudio*)userData;
    if (statusFlags) {
        v->errorcounter++;
        v->error_last = (uint16_t)statusFlags;
    }
    v->blockcounter++;
    v->lastblocksize = framesPerBuffer;

#ifndef PANEL_AUDIO_NONBUFFERED_CALLS
    uint32_t pos = v->audio_tail % (WIDAUDIO_HW_BUFFER_SIZE * 2);
    for (unsigned i = 0; i < framesPerBuffer; i++) {
        memcpy(
            &v->audio_in[pos * WIDAUDIO_HW_IN_CNANNELS],
            &((const WAudioT*)inputBuffer)[i * WIDAUDIO_HW_IN_CNANNELS],
            sizeof(WAudioT) * WIDAUDIO_HW_IN_CNANNELS
        );
        memcpy(
            &((WAudioT*)outputBuffer)[i * WIDAUDIO_HW_OUT_CNANNELS],
            &v->audio_out[pos * WIDAUDIO_HW_OUT_CNANNELS],
            sizeof(WAudioT) * WIDAUDIO_HW_OUT_CNANNELS
        );
        pos = (pos + 1) % (WIDAUDIO_HW_BUFFER_SIZE * 2);
    }
    v->audio_tail = v->audio_tail + framesPerBuffer;
#else // PANEL_AUDIO_NONBUFFERED_CALLS
    v->process_callback(inputBuffer, outputBuffer, framesPerBuffer);
#endif // PANEL_AUDIO_NONBUFFERED_CALLS
    return paContinue;
}

// it is possible to use multiple  
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

static void wAudioRedraw(void* wid) // TODO:
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
            if (WAUDIO_ACTIVE == v->status) {
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
#ifndef PANEL_AUDIO_NONBUFFERED_CALLS
    // TODO: time analysis
    while ((v->audio_tail - v->audio_head) > v->blocksize) {
        uint32_t current_app_pos = (v->audio_head) % (WIDAUDIO_HW_BUFFER_SIZE * 2);
        uint32_t new_app_pos = (v->audio_head + v->blocksize) % (WIDAUDIO_HW_BUFFER_SIZE * 2);
        if (new_app_pos < v->blocksize) {
            // data should be linear for the callback, so we use buffer extension and additional input de-wrapping
            // and output wrapping steps
            memcpy(
                &v->audio_in[WIDAUDIO_HW_BUFFER_SIZE * 2 * WIDAUDIO_HW_IN_CNANNELS],
                &v->audio_in[0],
                sizeof(WAudioT) * WIDAUDIO_HW_IN_CNANNELS * (new_app_pos + 1)
            );
            v->process_callback(
                &v->audio_in[current_app_pos * WIDAUDIO_HW_IN_CNANNELS],
                &v->audio_out[current_app_pos * WIDAUDIO_HW_OUT_CNANNELS],
                v->blocksize
            );
            memcpy(
                v->audio_out,
                &v->audio_out[WIDAUDIO_HW_BUFFER_SIZE * 2 * WIDAUDIO_HW_OUT_CNANNELS],
                sizeof(WAudioT) * WIDAUDIO_HW_OUT_CNANNELS * (new_app_pos + 1)
            );
        } else {
            v->process_callback(
                &v->audio_in[current_app_pos * WIDAUDIO_HW_IN_CNANNELS],
                &v->audio_out[current_app_pos * WIDAUDIO_HW_OUT_CNANNELS],
                v->blocksize
            );
        }
        v->audio_head = v->audio_head + v->blocksize;
    }
#endif // PANEL_AUDIO_NONBUFFERED_CALLS
}

static void wAudioTerminate(void* wid)
{
    WidgetAudio* v = (WidgetAudio*)wid;
    WAUDIO_PRINTF("\n wAudioTerminate");

    if (WAUDIO_OFF != v->status) {
        PaError err;
        if ((WAUDIO_ACTIVE == v->status) || (WAUDIO_ERROR == v->status)) {
            err = Pa_StopStream((PaStream*)v->instance);
            WAUDIO_PRINTF("\nPa_StopStream: %s", Pa_GetErrorText(err));
            // err = Pa_AbortStream( stream ); // immediately
            err = Pa_CloseStream((PaStream*)v->instance);
            WAUDIO_PRINTF("\nPa_CloseStream: %s", Pa_GetErrorText(err));
        }
        portaudioTerminateGlobal();
    }
}

static void wAudioMouseMove(void* wid, WidgetTouchData* d, unsigned touch_elements) // TODO
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
static void wAudioMouseClick(void* wid, WidgetTouchData* d) // TODO
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

#ifndef PANEL_AUDIO_NONBUFFERED_CALLS
    #define BLOCKSIZE WIDAUDIO_HW_BUFFER_SIZE
#else
    #define BLOCKSIZE v->blocksize
#endif

void wAudioInit(
    WidgetAudio* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t samplerate,
    uint32_t blocksize,
    PanelAudioProcessCallbackT* process_callback)
{
    memset(v, 0, sizeof(WidgetAudio));

    if (dev_name_in)
        SDL_strlcpy(v->name_in, dev_name_in, WIDAUDIO_NAMELENGTH);
    else
        v->name_in[0] = 0;
    if (dev_name_out)
        SDL_strlcpy(v->name_out, dev_name_out, WIDAUDIO_NAMELENGTH);
    else
        v->name_out[0] = 0;
    v->samplerate = samplerate;
 
#ifndef PANEL_AUDIO_NONBUFFERED_CALLS
    SDL_assert(blocksize < WIDAUDIO_HW_BUFFER_SIZE);
#endif
    v->blocksize = blocksize;
    v->process_callback = process_callback;

    widgetInit(&v->v, (void*)v, &wAudioApi, x, y, 40, 19, 1, rend);
    v->status = WAUDIO_OFF;
    v->instance = 0;
    v->errorcounter = v->errorcounter_prev = v->error_last = 0;
    v->blockcounter = 0;

    WAUDIO_PRINTF("\n wAudioInit");

    PaError err;
    portaudioInitGlobal(); // this will modify global portaudio_status
    if (paNoError == portaudio_status) {
        if ((0 == v->name_in[0]) && (0 == v->name_out[0])) {
            // names are not defined, use default devices
            err = Pa_OpenDefaultStream(
                (PaStream**)&v->instance,
                WIDAUDIO_HW_IN_CNANNELS,
                WIDAUDIO_HW_OUT_CNANNELS,
                sizeof(WAudioT) == 2 ? paInt16 : paInt32,
                v->samplerate,
                BLOCKSIZE,
                pa_callback,
                v
            );
            WAUDIO_PRINTF("\nPa_OpenDefaultStream: %s", Pa_GetErrorText(err));
            if (0 == err) {
                err = Pa_StartStream((PaStream*)v->instance);
                WAUDIO_PRINTF("\nPa_StartStream: %s", Pa_GetErrorText(err));
            }
            v->status = (0 == err) ? WAUDIO_ACTIVE : WAUDIO_ERROR;
            // get names
            const PaDeviceIndex din = Pa_GetDefaultInputDevice();
            const PaDeviceInfo* din_info = Pa_GetDeviceInfo(din);
            const PaDeviceIndex dout = Pa_GetDefaultOutputDevice();
            const PaDeviceInfo* dout_info = Pa_GetDeviceInfo(dout);
            SDL_strlcpy(v->name_in, din_info->name, WIDAUDIO_NAMELENGTH);
            SDL_strlcpy(v->name_out, dout_info->name, WIDAUDIO_NAMELENGTH);
            WAUDIO_PRINTF(
                "\n    in: %d, latency: %g, channels: %d, name: %s",
                din, din_info->defaultLowInputLatency, din_info->maxInputChannels, din_info->name);
            WAUDIO_PRINTF(
                "\n    out: %d, latency: %g, channels: %d, name: %s",
                dout, dout_info->defaultLowOutputLatency, dout_info->maxOutputChannels, dout_info->name);
        } else {
            // names were defined. let's find them in devices list
            v->status = WAUDIO_NOTFOUND;
            const PaDeviceIndex num_devices = Pa_GetDeviceCount();

            if (num_devices > 0) {
                PaStreamParameters stream_in;
                stream_in.device = -1;
                stream_in.channelCount = WIDAUDIO_HW_IN_CNANNELS;
                stream_in.sampleFormat = sizeof(WAudioT) == 2 ? paInt16 : paInt32;
                stream_in.suggestedLatency = 10;
                stream_in.hostApiSpecificStreamInfo = NULL;
                PaStreamParameters stream_out;
                stream_out.device = -1;
                stream_out.channelCount = WIDAUDIO_HW_OUT_CNANNELS;
                stream_out.sampleFormat = sizeof(WAudioT) == 2 ? paInt16 : paInt32;
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
                    v->status = WAUDIO_NOTFOUND;
                    WAUDIO_PRINTF("\n Not found");
                } else {
                    WAUDIO_PRINTF("\nFound in: %d, out: %d", stream_in.device, stream_out.device);
                    const PaDeviceInfo* din_info = Pa_GetDeviceInfo(stream_in.device);
                    const PaDeviceInfo* dout_info = Pa_GetDeviceInfo(stream_out.device);
                    SDL_strlcpy(v->name_in, din_info->name, WIDAUDIO_NAMELENGTH - 2);
                    SDL_strlcpy(v->name_out, dout_info->name, WIDAUDIO_NAMELENGTH - 2);
                    err = Pa_OpenStream((PaStream**)&v->instance, &stream_in, &stream_out, v->samplerate, BLOCKSIZE, paNoFlag, pa_callback, v);
                    WAUDIO_PRINTF("\nPa_OpenStream: %s", Pa_GetErrorText(err));
                    if (0 == err) {
                        err = Pa_StartStream((PaStream*)v->instance);
                        WAUDIO_PRINTF("\nPa_StartStream: %s", Pa_GetErrorText(err));
                    }
                    v->status = (0 == err) ? WAUDIO_ACTIVE : WAUDIO_ERROR;
                }
            }
        }
    }
}
