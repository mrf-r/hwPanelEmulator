// vibecoded in Anthropic Claude Opus 4.1
#ifndef FATFS_WRAPPER_H
#define FATFS_WRAPPER_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* FatFs return values */
typedef enum {
    FR_OK = 0,
    FR_DISK_ERR,
    FR_INT_ERR,
    FR_NOT_READY,
    FR_NO_FILE,
    FR_NO_PATH,
    FR_INVALID_NAME,
    FR_DENIED,
    FR_EXIST,
    FR_INVALID_OBJECT,
    FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE,
    FR_NOT_ENABLED,
    FR_NO_FILESYSTEM,
    FR_MKFS_ABORTED,
    FR_TIMEOUT,
    FR_LOCKED,
    FR_NOT_ENOUGH_CORE,
    FR_TOO_MANY_OPEN_FILES,
    FR_INVALID_PARAMETER
} FRESULT;

/* File access modes */
#define FA_READ             0x01
#define FA_WRITE            0x02
#define FA_OPEN_EXISTING    0x00
#define FA_CREATE_NEW       0x04
#define FA_CREATE_ALWAYS    0x08
#define FA_OPEN_ALWAYS      0x10
#define FA_OPEN_APPEND      0x30

/* File attributes */
#define AM_RDO  0x01    /* Read only */
#define AM_HID  0x02    /* Hidden */
#define AM_SYS  0x04    /* System */
#define AM_DIR  0x10    /* Directory */
#define AM_ARC  0x20    /* Archive */

/* Maximum open files */
#define MAX_OPEN_FILES 256

/* FIL structure */
typedef struct {
    FILE* fp;           /* Standard C FILE pointer */
    char fname[256];    /* File name */
    uint32_t fsize;     /* File size */
    uint32_t fptr;      /* File pointer */
    uint8_t flag;       /* File status flags */
    uint8_t err;        /* Abort flag */
} FIL;

/* DIR structure */
typedef struct {
    void* dir;          /* Directory stream pointer */
    char path[256];     /* Directory path */
    int index;          /* Current index */
} DIR_WRAPPER;

/* FILINFO structure */
typedef struct {
    uint32_t fsize;     /* File size */
    uint16_t fdate;     /* Last modified date */
    uint16_t ftime;     /* Last modified time */
    uint8_t fattrib;    /* File attributes */
    char fname[256];    /* File name */
    char altname[13];   /* Alternative file name */
} FILINFO;

/* FATFS structure */
typedef struct {
    uint8_t fs_type;    /* File system type */
    uint8_t drv;        /* Drive number */
    char path[256];     /* Root path */
    uint8_t mounted;    /* Mount status */
} FATFS;

/* Function prototypes */
FRESULT f_mount(FATFS* fs, const char* path, uint8_t opt);
FRESULT f_unmount(const char* path);
FRESULT f_open(FIL* fp, const char* path, uint8_t mode);
FRESULT f_close(FIL* fp);
FRESULT f_read(FIL* fp, void* buff, uint32_t btr, uint32_t* br);
FRESULT f_write(FIL* fp, const void* buff, uint32_t btw, uint32_t* bw);
FRESULT f_lseek(FIL* fp, uint32_t ofs);
uint32_t f_tell(FIL* fp);
uint32_t f_size(FIL* fp);
FRESULT f_sync(FIL* fp);
FRESULT f_truncate(FIL* fp);

FRESULT f_opendir(DIR_WRAPPER* dp, const char* path);
FRESULT f_closedir(DIR_WRAPPER* dp);
FRESULT f_readdir(DIR_WRAPPER* dp, FILINFO* fno);
FRESULT f_mkdir(const char* path);
FRESULT f_unlink(const char* path);
FRESULT f_rename(const char* path_old, const char* path_new);
FRESULT f_stat(const char* path, FILINFO* fno);

FRESULT f_gets(char* buff, int len, FIL* fp);
FRESULT f_puts(const char* str, FIL* fp);
FRESULT f_printf(FIL* fp, const char* fmt, ...);

FRESULT f_chdir(const char* path);
FRESULT f_chdrive(const char* path);
FRESULT f_getcwd(char* buff, uint32_t len);

/* Utility macros */
#define f_eof(fp) ((fp)->fptr >= (fp)->fsize)
#define f_error(fp) ((fp)->err)

#endif /* FATFS_WRAPPER_H */
