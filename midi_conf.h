/*
Copyright (C) 2024 Eugene Chernyh (mrf-r)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _MIDI_CONF_H
#define _MIDI_CONF_H

#include <SDL.h>
#define MIDI_ASSERT SDL_assert

// CN should be defined in userpanel ?
//
typedef enum {
    MIDI_CN_LOCALPANEL = 0,
    MIDI_CN_USB_DEVICE,
    MIDI_CN_USB_HOST,
    MIDI_CN_UART1,
    MIDI_CN_UART2,
    MIDI_CN_UART3,
    MIDI_CN_TOTAL,
} MidiCnEn;

#define MIDI_GET_CLOCK() SDL_GetTicks()
#define MIDI_CLOCK_RATE 1000

#define MIDI_ATOMIC_START()
#define MIDI_ATOMIC_END()

#endif // _MIDI_CONF_H
