#ifndef __WID_AUDIO_H
#define __WID_AUDIO_H

#include "widget.h"
#include <portaudio.h>

#define VIDAUDIO_NAMELENGTH 32

typedef enum {
    VAUDIO_OFF = 0,
    VAUDIO_NOTFOUND,
    VAUDIO_ERROR,
    VAUDIO_ACTIVE
} WidAudioStatus;

typedef struct {
    Widget v;
    uint8_t pointed;
    char name_in[VIDAUDIO_NAMELENGTH];
    char name_out[VIDAUDIO_NAMELENGTH];
    uint32_t samplerate;
    uint32_t blocksize;
    void* instance;
    uint8_t status;
    uint16_t lastblocksize;
    uint16_t blockcounter;
    uint16_t errorcounter;
    uint16_t error_last;
    uint16_t errorcounter_prev;
} WidgetAudio;

// TODO: channels and bits?
#define WID_AUDIO_CALLBACK_DEFINE(pa_callback_name, simplified_callback)                     \
    static int pa_callback_name(                                                             \
        const void* inputBuffer,                                                             \
        void* outputBuffer,                                                                  \
        unsigned long framesPerBuffer,                                                       \
        const PaStreamCallbackTimeInfo* timeInfo,                                            \
        PaStreamCallbackFlags statusFlags,                                                   \
        void* userData)                                                                      \
    {                                                                                        \
        (void)timeInfo; /* TODO: do something with timeInfo? */                              \
        WidgetAudio* v = (WidgetAudio*)userData;                                             \
        if (statusFlags) {                                                                   \
            v->errorcounter++;                                                               \
            v->error_last = (uint16_t)statusFlags;                                           \
        }                                                                                    \
        v->blockcounter++;                                                                   \
        v->lastblocksize = framesPerBuffer;                                                  \
        simplified_callback((int16_t*)inputBuffer, (int16_t*)outputBuffer, framesPerBuffer); \
        return paContinue;                                                                   \
    }

void wAudioInit(
    WidgetAudio* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t samplerate,
    uint32_t blocksize,
    PaStreamCallback* pa_callback_name);

#endif // __WID_AUDIO_H