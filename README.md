# SDL2 hardware emulator
for experimenting with different layouts, MIDI sequencing and audio synthesis.

# dependencies:
- make gcc pkgconf
- SDL2 portmidi libserialport portaudio

# TODO:
- primary:
    - midi connect internal
    - midi widget (mostly for restarting it) and actual control
- secondary
    - audio widget
    - additional configs load (midi ports and audio selection, color scheme, size/scale?)
        - runtme panel init from config (simplified functionality) ?
    - always on top switch?
    - fatfs integration (?)
    - performance analysis (?)
