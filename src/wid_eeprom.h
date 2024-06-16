#ifndef __WID_EEPROM_H
#define __WID_EEPROM_H

#include <stdint.h>
#include "SDL.h"

typedef struct {
    const char* name;
    const uint16_t size;
    const char* filepath;
} GadgetEeprom;

void wEepromInit(GadgetEeprom* v);

#endif // __WID_EEPROM_H