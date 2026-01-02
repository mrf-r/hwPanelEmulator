
#include "wid_eeprom.h"
#include <SDL.h>
#include <stdio.h>
#include <time.h>
#define WEEPROM_PRINTF printf
// #define WEEPROM_PRINTF(...)

#define EEPROM_HEADER_SIZE 256
#define EEPROM_FILEID_SIZE 32
#define EEPROM_NAME_SIZE 64
#define EEPROM_FILEID "hwPanelEmulator_EEPROM"

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dummy;
} EepromTime;

typedef union {
    uint8_t array[EEPROM_HEADER_SIZE];
    struct
    {
        char file_id[EEPROM_FILEID_SIZE];
        char name[EEPROM_NAME_SIZE];
        uint32_t header_size;
        uint32_t data_size;
        uint32_t write_count;
        EepromTime created;
        EepromTime modified;
    };
} EepromHeader;

__attribute__((unused)) static const EmuEeprom* eeprom;

void wEepromSelect(const EmuEeprom* const e)
{
    SDL_assert(e);
    eeprom = e;
}

static void eepromFillTime(EepromTime* t)
{
    time_t ept;
    time(&ept);
    struct tm* timenow = localtime(&ept);
    t->year = timenow->tm_year;
    t->month = timenow->tm_mon;
    t->day = timenow->tm_mday;
    t->hour = timenow->tm_hour;
    t->minute = timenow->tm_min;
    t->second = timenow->tm_sec;
}

static void eepromHeaderInit(EepromHeader* h)
{
    SDL_memset(h->array, 0x0, EEPROM_HEADER_SIZE);
    SDL_strlcpy(h->file_id, EEPROM_FILEID, EEPROM_FILEID_SIZE);
    SDL_strlcpy(h->name, eeprom->name, EEPROM_NAME_SIZE);
    h->data_size = eeprom->size;
    h->header_size = EEPROM_HEADER_SIZE;
}

static BspResult eepromCreateNew()
{
    BspResult result = BSP_OK;
    WEEPROM_PRINTF("\nFile create attempt");
    SDL_RWops* fops = SDL_RWFromFile(eeprom->filepath, "w+b");
    if (NULL == fops) {
        WEEPROM_PRINTF("\nSDL_RWFromFile(create): %s", SDL_GetError());
        result = BSP_FAIL;
    } else {
        // write header
        EepromHeader header;
        eepromHeaderInit(&header);
        eepromFillTime(&header.created);
        header.created.dummy = 'C';
        if (1 != SDL_RWwrite(fops, header.array, EEPROM_HEADER_SIZE, 1)) {
            WEEPROM_PRINTF("\nSDL_RWwrite(header): %s", SDL_GetError());
            result = BSP_FAIL;
        }
        // write init data
        for (unsigned i = 0; i < eeprom->size; i++) {
            static const uint8_t init_data = 0xFF;
            if (1 != SDL_RWwrite(fops, &init_data, 1, 1)) {
                result = BSP_FAIL;
            }
        }
    }
    if (SDL_RWclose(fops)) {
        WEEPROM_PRINTF("\nSDL_RWclose(create): %s", SDL_GetError());
    }
    return result;
}

static BspResult eepromCheckValidity(SDL_RWops* fops)
{
    BspResult result = BSP_OK;
    EepromHeader header;
    eepromHeaderInit(&header);
    EepromHeader hread;
    if (1 != SDL_RWread(fops, hread.array, EEPROM_HEADER_SIZE, 1)) {
        WEEPROM_PRINTF("\nSDL_RWread(hcheck): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    unsigned cmp_len = EEPROM_FILEID_SIZE + EEPROM_NAME_SIZE + 4 + 4;
    if (SDL_memcmp(header.array, hread.array, cmp_len)) {
        WEEPROM_PRINTF("\nHeader mismatch!");
        // TODO: messagebox
        result = BSP_FAIL;
    }
    int64_t fend = SDL_RWseek(fops, 0, RW_SEEK_END);
    if (fend < 0) {
        WEEPROM_PRINTF("\nSDL_RWseek(end): %s", SDL_GetError());
        result = BSP_FAIL;
    } else {
        if ((eeprom->size + EEPROM_HEADER_SIZE) != fend) {
            WEEPROM_PRINTF("\nSize mismatch: %d", (int)fend);
            // TODO: messagebox
            result = BSP_FAIL;
        } else {
            WEEPROM_PRINTF("\nEEPROM file is OK!");
        }
    }
    return result;
}

static BspResult eepromFileCheck()
{
    BspResult result = BSP_OK;
    // file check
    SDL_RWops* fops = SDL_RWFromFile(eeprom->filepath, "rb");
    if (NULL == fops) {
        WEEPROM_PRINTF("\nSDL_RWFromFile(fcheck): %s", SDL_GetError());
        if (BSP_OK != eepromCreateNew()) {
            result = BSP_FAIL;
        }
        fops = SDL_RWFromFile(eeprom->filepath, "rb");
    } else {
        // check header match
        if (BSP_OK != eepromCheckValidity(fops)) {
            result = BSP_FAIL;
        }
    }
    if (SDL_RWclose(fops)) {
        WEEPROM_PRINTF("\nSDL_RWclose(fcheck): %s", SDL_GetError());
    }
    return result;
}

BspResult bspEepromRead(uint32_t address, uint8_t* dest, uint32_t length)
{
    SDL_assert(eeprom);
    BspResult result = BSP_OK;
    if (BSP_OK != eepromFileCheck()) {
        return BSP_FAIL;
    }

    SDL_RWops* fops = SDL_RWFromFile(eeprom->filepath, "rb");
    if (NULL == fops) {
        WEEPROM_PRINTF("\nSDL_RWFromFile(read): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (0 > SDL_RWseek(fops, EEPROM_HEADER_SIZE + address, RW_SEEK_SET)) {
        WEEPROM_PRINTF("\nSDL_RWseek(read): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (1 != SDL_RWread(fops, dest, length, 1)) {
        WEEPROM_PRINTF("\nSDL_RWread(read): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (SDL_RWclose(fops)) {
        WEEPROM_PRINTF("\nSDL_RWclose(read): %s", SDL_GetError());
    }
    return result;
}

BspResult bspEepromWrite(uint32_t address, uint8_t* data, uint32_t length)
{
    SDL_assert(eeprom);
    BspResult result = BSP_OK;
    if (BSP_OK != eepromFileCheck()) {
        return BSP_FAIL;
    }

    SDL_RWops* fops = SDL_RWFromFile(eeprom->filepath, "r+b");
    if (NULL == fops) {
        WEEPROM_PRINTF("\nSDL_RWFromFile(write): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    EepromHeader header;
    if (1 != SDL_RWread(fops, header.array, EEPROM_HEADER_SIZE, 1)) {
        WEEPROM_PRINTF("\nSDL_RWread(w header): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    eepromFillTime(&header.modified);
    header.modified.dummy = 'E';
    header.write_count++;
    if (0 > SDL_RWseek(fops, 0, RW_SEEK_SET)) {
        WEEPROM_PRINTF("\nSDL_RWseek(w header): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (1 != SDL_RWwrite(fops, header.array, EEPROM_HEADER_SIZE, 1)) {
        WEEPROM_PRINTF("\nSDL_RWwrite(w header): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (0 > SDL_RWseek(fops, EEPROM_HEADER_SIZE + address, RW_SEEK_SET)) {
        WEEPROM_PRINTF("\nSDL_RWseek(w data): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (1 != SDL_RWwrite(fops, data, length, 1)) {
        WEEPROM_PRINTF("\nSDL_RWwrite(w data): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (SDL_RWclose(fops)) {
        WEEPROM_PRINTF("\nSDL_RWclose(read): %s", SDL_GetError());
    }
    return result;
}
