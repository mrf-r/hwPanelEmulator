#ifndef __WID_EEPROM_H
#define __WID_EEPROM_H

#include <stdint.h>
#include <SDL.h>

typedef enum {
    BSP_OK = 0,
    BSP_FAIL,
} BspResult;

typedef struct {
    const char* name;
    const uint16_t size;
    const char* filepath;
} EmuEeprom;

void wEepromSelect(const EmuEeprom* const e);
BspResult bspEepromRead(uint32_t address, uint8_t* dest, uint32_t length);
BspResult bspEepromWrite(uint32_t address, uint8_t* data, uint32_t length);

#endif // __WID_EEPROM_H