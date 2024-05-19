# SDL2 hardware emulator

![](slseq.png "example")

Cross platform project for experimenting with different MIDI control panel layouts, sequencing and audio synthesis. Compatible with Arduino MIDI over serial.

Tested in Linux and MINGW64.

The idea is to have Arduino-like simplicity in building control surface in pure c. All actual panel code located in `user/` folder by default. You can change it by `make DIR_USER=my_superbest_panel`. There are basically only 2 callbacks:
- `void panelConstruct(SDL_Renderer* rend)` - in which you must init various widgets you need on your surface
- `void panelLoop(uint32_t clock)` - in which you must handle those widgets (the application part), perform midi processing and routing and other logic you need
- `void panelProcessAudio(*in, *out)` - TBD

## widgets
All widgets generates events in form of midi messages. You can differentiate them by CN (cable number) field in message structure (for details see my mbwmidi library and USB MIDI class specs). All widgets have leds (RGB). For the displays led acts as a backlight color.
- Button - can be pressed by the mouse or keyboard. Will generate events on press and release
- Encoder - can be turned by the mouse drag, scroll wheels, or 2 keyboard keys for inc and dec. Will generate events with relative signed values depending on the travel distance.
- Pot - emulation of the common microcontroller potentiometer + ADC circuit, will generate high resolution midi values with some noise. can be dragged by mouse or by scroll wheels. Can be locked in case of function or patch change (part of mbwmidi library).
- Displays. They are not generate any events. Dedicated api will be created for each defined instance. Graphic ones based on minimalgraphics library. You can press on the display to make a screenshot.
    - character - standart 1602 type with cgram.
    - mono - graphic lcd or oled
    - multi - tft or 4bit oled (by default RGB565 noise is added)
- midi - !only one instance can be used! portmidi and libserialport connection to outside. It displays number of received and sent messages and can be clicked for reinit connection (if you unplug your USB-MIDI device, it can not be seen by sw). You need to specify names of the input and output devices you wanna to use. Only part of the string is enough. I plan to put that in some text configuration file, but at the moment it should be hardcoded. You also need to specify baudrates:
    - For serial devices an actual port setting should be specified (i.e. 9600 or 115200 or 31250..)
    - For both midi and serial devices flow limiter value should be specified (virtual baud). You are free to experiment with different rates. For example, if you are using some old synths, you can lower the baud from 31250 to 10000 or even less. Minimal virtual baud is 10 bits per second.
Should be compatible with Arduino MIDI over serial.
- audio - !only one instance can be used! based on portaudio library. In case you want to experiment with synthesis. It can be inited with NULL instead of device names to select default devices. You will need to provide audio callback for block calculation `void audioBufferProcessCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length)`. For now it is called from portaudio library, so all thread safety is on you. I'm thinking about calling it from widget process, but it will introduce additional latency.

## future widgets
- file - in case you want to experiment with patches or audio samples

# dependencies:
- make gcc pkgconf
- SDL2 portmidi libserialport portaudio (probably with -dev suffix, TODO check on linux)
- and do not forget to `git submodule update --init --recursive`

# TODO:
- primary:
    - makefile to split build process? and to allow multiple files for user panel.
    - COORDINATES AND SCALES ARE MESS
- secondary
    - default audio device is not the one with lowest latency (on Win)
    - does audio work in a systems with no input(output) devices?
    - performance analysis (?)
    - filesystem integration (?)
    - eeprom emulation
    - encoder press (if distance < thrsh on mouse release)?
    - multiple keyboard keys for one element
    - additional configs load (midi ports and audio selection, color scheme, size/scale?)
        - runtme panel init from config (simplified functionality) ?
    - always on top switch?
    - multiple midi IO (within one widget)
    - multiple audio IO
    - audio SRC for weird samplerates (arbitraty between 10k and 384k)
