#ifndef __WID_AUDIO_H
#define __WID_AUDIO_H

#include "widget.h"
    
#define VIDAUDIO_NAMELENGTH 32
// #define PANEL_AUDIO_NONBUFFERED_CALLS

typedef enum {
    VAUDIO_OFF = 0,
    VAUDIO_NOTFOUND,
    VAUDIO_ERROR,
    VAUDIO_ACTIVE
} WidAudioStatus;

typedef int32_t WAudioT; // TODO: do something with paInt16-paInt32 selections
typedef void PanelAudioProcessCallbackT(const WAudioT* const input, WAudioT* const output, const unsigned frameCount);

 // TODO: PANEL_AUDIO_BUFFER_SIZE configurable
#ifndef PANEL_AUDIO_HW_BUFFER_SIZE
#define PANEL_AUDIO_HW_BUFFER_SIZE 2048
#endif
#define PANEL_AUDIO_HW_IN_CNANNELS 2
#define PANEL_AUDIO_HW_OUT_CNANNELS 2

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
#ifndef PANEL_AUDIO_NONBUFFERED_CALLS
    // 3 buffers for unaligned
    WAudioT audio_out[PANEL_AUDIO_HW_BUFFER_SIZE * PANEL_AUDIO_HW_OUT_CNANNELS * 3];
    WAudioT audio_in[PANEL_AUDIO_HW_BUFFER_SIZE * PANEL_AUDIO_HW_IN_CNANNELS * 3];
    volatile uint32_t audio_head;
    volatile uint32_t audio_tail;
#endif
    PanelAudioProcessCallbackT* process_callback;
} WidgetAudio;

void wAudioInit(
    WidgetAudio* v,
    uint16_t x,
    uint16_t y,
    SDL_Renderer* rend,
    const char* dev_name_in,
    const char* dev_name_out,
    uint32_t samplerate, // same for hw and app
    uint32_t blocksize, // application blocksize may differ from hw buffer
    PanelAudioProcessCallbackT* process_callback);

#endif // __WID_AUDIO_H