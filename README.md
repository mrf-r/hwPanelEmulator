# SDL2 hardware emulator

Cross platform project for experimenting with different MIDI control panel layouts, sequencing and audio synthesis. An attempt has been made to make the control as convenient as possible on a computer. Simultaneous input from the keyboard, mouse or multi-touch panel is supported.Compatible with Arduino MIDI over serial.

The idea is to have Arduino-like simplicity in building control surfaces and dsp code in C/C++.

- `#include "panel.h"`
- `void panelConstruct(SDL_Renderer* rend)` - in which you must init various widgets you need on your surface
- `void panelLoop(uint32_t clock)` - in which you must handle those widgets (the application part), perform midi processing and routing and other logic you need. NOTE: call rate may be less than 60 Hz!!!
- `void synthAudioCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length)` - A simplified callback called by PortAudio. The call rate depends on the block size and sample rate parameters. This method should only contain a buffer access, while the main audio processing should preferably be performed in the main loop (panelLoop). The buffer size should therefore take into account the low frequency of the panelLoop - approximately 1024 samples for 48 kHz.

It is better to look at the implementation details in the https://github.com/mrf-r/hwpeTestProj. It will likely become outdated over time. I'll try to move the documentation here.

### Components:

- Button - can be pressed or drag-pressed by the mouse, keyboard or touch. Will generate events on press and release in the form of midi messages. Can be differentiated from regular midi messages by .cn (cable number) field in message structure (see mbwmidi lib and USB MIDI class 1 specs). Widgets have RGB leds. For the displays led acts as a backlight color.
- Encoder - touch and drag (y-coarse, x-fine), or scroll wheel over it. Will generate events with relative signed values depending on the travel distance.
- Pot - emulation of the common microcontroller potentiometer + ADC circuit, will generate high resolution midi values with some noise. can be dragged or scroll wheel-ed. Can be locked in case of function or patch change (part of mbwmidi library).
- Displays. Does not generate any events. Dedicated api will be created for each defined instance. Graphic ones based on minimalgraphics library. You can press on the display to make a screenshot.
    - character - standart 1602 type with cgram.
    - mono - graphic lcd or oled
    - multi - tft or 4bit oled (by default RGB565 with dither)
- midi - !only one instance can be used! portmidi and libserialport connection to outside. It displays number of received and sent messages and can be clicked for reinit connection (if you unplug your USB-MIDI device, it can not be seen by sw). You need to specify names of the input and output devices you wanna to use. Only part of the string is enough. I plan to put that in some text configuration file, but at the moment it should be hardcoded. You also need to specify baudrates:
    - For serial devices an actual port setting should be specified (i.e. 9600 or 115200 or 31250..)
    - For both midi and serial devices flow limiter value should be specified (virtual baud). You are free to experiment with different rates. For example, if you are using some old synths, you can lower the baud from 31250 to 10000 or even less. Minimal virtual baud is 10 bits per second.
Should be compatible with Arduino MIDI over serial.
- audio - based on portaudio library. In case you want to experiment with synthesis. It can be initialized with NULL instead of device names to select default devices. Portaudio callback for each audio connection should be created using `WID_AUDIO_CALLBACK_DEFINE` macro. When creating it, you need to specify the name of the callback to be created, which will be used when creating the widget, as well as the block handling method it will call. At the moment the call is made from portaudio library, so all thread safety is on you. I am thinking about calling it from widget process, but it will cause additional latency.
- eeprom - just bsp read and write methods, which is about the same as what you probably get on hw. You may have multiple chips. Make sure you select one with `void wEepromSelect(const EmuEeprom* const e)` before calling read or write. **!! The file name must not contain folders !!**
- ff - [FatFS](https://www.elm-chan.org/fsw/ff/) emulator. Disk `0:/` is `{cwd}/ff_sd/`, disk `1:/` - `{cwd}/ff_ext/`.

### Dependencies

- [mbwmidi](https://github.com/mrf-r/mbwmidi), [graphics](https://github.com/mrf-r/minimalgraphics) - expected to be part of the project
- SDL2, portmidi, libserialport, portaudio-2.0 - used by this module

### TODO:

- pot/encoder rate limiter
- midi select device or use default device
- test midi over serial
- 

