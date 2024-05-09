#include "panel_conf.h"
#include "panel.h"

void panelConstruct(SDL_Renderer* rend) { }
void panelLoop(uint32_t ms) { }
void wButtonMidiSend(uint8_t midictrl, uint8_t value) { }
void wPotMidiSend(uint8_t midictrl, uint16_t value) { }
void wEncMidiSend(uint8_t midictrl, int8_t value) { }