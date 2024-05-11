# SDL2 hardware emulator

![](slseq.png "example")

Cross platform project for experimenting with different MIDI control panel layouts, sequencing and audio synthesis.
Tested in Linux and MINGW64.

The idea is to have Arduino-like simplicity in building control surface in pure c. All actual panel code located in `user/` folder by default. You can change it by `make DIR_USER=my_superbest_panel`. There are basically only 2 callbacks:
- `void panelConstruct(SDL_Renderer* rend)` - in which you must init various widgets you need on your surface
- `void panelLoop(uint32_t ms)` - in which you must handle those widgets (the application part), perform midi processing and routing and other logic you need

## widgets
All widgets generates events in form of midi messages. You can differentiate them by CN (cable number) field in message structure (for details see my mbwmidi library and USB MIDI class specs).
- Button - can be pressed by the mouse or keyboard. Will generate events on press and release
- Encoder - can be turned by the mouse drag, scroll wheels, or 2 keyboard keys for inc and dec. Will generate events with relative signed values depending on the travel distance.
- Pot - emulation of the common microcontroller potentiometer + ADC circuit, will generate high resolution midi values with some noise. can be dragged by mouse or by scroll wheels. Can be locked in case of function or patch change (part of mbwmidi library).
- Displays. They are not generate any events. Dedicated api will be created for each defined instance. Graphic ones based on minimalgraphics library.
    - character - standart 1602 type with cgram.
    - mono - graphic lcd or oled
    - multi - tft or 4bit oled (by default RGB565 noise is added)

## future widgets
- midi - portmidi and libserialport connection to outside. Should be compatible with Arduino MIDI over serial.
- audio - portaudio in case you want to experiment with synthesis
- file - in case you want to experiment with patches or audio samples


# dependencies:
- make gcc pkgconf
- SDL2 portmidi libserialport portaudio

# TODO:
- primary:
    - COORDINATES AND SCALES ARE MESS
    - midi connect internal
    - midi widget (mostly for restarting it) and actual control
- secondary
    - encoder press (if distance < thrsh on mouse release)?
    - multiple keyboard keys for one element
    - makefile to split build process?
    - audio widget
    - additional configs load (midi ports and audio selection, color scheme, size/scale?)
        - runtme panel init from config (simplified functionality) ?
    - always on top switch?
    - fatfs integration (?)
    - performance analysis (?)
