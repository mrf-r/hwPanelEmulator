
#include "wid_eeprom.h"
#include "SDL.h"
// #include "file"
#include "stdio.h"
#include "time.h"
#define WEEPROM_PRINTF printf
// #define WEEPROM_PRINTF(...)

typedef enum {
    BSP_OK = 0,
    BSP_FAIL,
} BspResult;

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

static void wEepromFillTime(EepromTime* t)
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

static GadgetEeprom* eeprom;

void eepromSelect(GadgetEeprom* e)
{
    SDL_assert(e);
    eeprom = e;
}

BspResult eepromRead(uint32_t address, uint8_t* dest, uint32_t length)
{
    BspResult result = BSP_OK;
    EepromHeader header;
    SDL_memset(header.array, 0x0, EEPROM_HEADER_SIZE);
    SDL_strlcpy(header.file_id, EEPROM_FILEID, EEPROM_FILEID_SIZE);
    SDL_strlcpy(header.name, eeprom->name, EEPROM_NAME_SIZE);
    header.data_size = eeprom->size;
    header.header_size = EEPROM_HEADER_SIZE;
    // file check
    SDL_RWops* fops = SDL_RWFromFile(eeprom->filepath, "rb");
    if (NULL == fops) {
        WEEPROM_PRINTF("\nSDL_RWFromFile(read): %s", SDL_GetError());
        WEEPROM_PRINTF("\nFile create attempt");
        fops = SDL_RWFromFile(eeprom->filepath, "w+b");
        if (NULL == fops) {
            WEEPROM_PRINTF("\nSDL_RWFromFile(create): %s", SDL_GetError());
            result = BSP_FAIL;
        } else {
            // write header
            wEepromFillTime(&header.created);
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
    } else {
        // check header match
        EepromHeader hread;
        if (1 != SDL_RWread(fops, hread.array, EEPROM_HEADER_SIZE, 1)) {
            WEEPROM_PRINTF("\nSDL_RWread(header): %s", SDL_GetError());
            result = BSP_FAIL;
        }
        unsigned cmp_len = EEPROM_FILEID_SIZE + EEPROM_NAME_SIZE + 4 + 4;
        if (SDL_memcmp(header.array, hread.array, cmp_len)) {
            WEEPROM_PRINTF("\nHeader mismatch!");
            result = BSP_FAIL;
        }
        int64_t fend = SDL_RWseek(fops, 0, RW_SEEK_END);
        if (fend < 0) {
            WEEPROM_PRINTF("\nSDL_RWseek(end): %s", SDL_GetError());
            result = BSP_FAIL;
        } else {
            if ((eeprom->size + EEPROM_HEADER_SIZE) != fend) {
                WEEPROM_PRINTF("\nSize mismatch: %d", (int)fend);
                result = BSP_FAIL;
            } else {
                WEEPROM_PRINTF("\nEEPROM file is OK!");
            }
        }
    }
    // actual read
    if (0 > SDL_RWseek(fops, EEPROM_HEADER_SIZE + address, RW_SEEK_SET)) {
        WEEPROM_PRINTF("\nSDL_RWseek(start): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (1 != SDL_RWread(fops, dest, length, 1)) {
        WEEPROM_PRINTF("\nSDL_RWread(data): %s", SDL_GetError());
        result = BSP_FAIL;
    }
    if (SDL_RWclose(fops)) {
        WEEPROM_PRINTF("\nSDL_RWclose: %s", SDL_GetError());
    }
    return result;
}

BspResult eepromWrite(uint32_t address, uint8_t* dest, uint32_t length)
{
    // open
    // write
    // close
    // time()
}
