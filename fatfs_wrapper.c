#include "fatfs_wrapper.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <SDL.h>
#include "panel_conf.h"

#ifndef FF_VOLUMES
#define FF_VOLUMES 2
#endif
#ifndef FF_VOLUME_ROOT
#define FF_VOLUME_ROOT {"./ff_sd", "./ff_ext"}
// TODO: SDL_GetBasePath
#endif

#ifndef FF_PRINTF
#define FF_PRINTF(...)
#endif

static const char *program_root[FF_VOLUMES] = FF_VOLUME_ROOT;

/* Static storage for mounted filesystems */
static FATFS mounted_fs[FF_VOLUMES]; // TODO: pointer

/* Helper function to translate FatFs paths to local filesystem paths */
static const char* translate_path(const char* fatfs_path, char* buffer, size_t buffer_size) {
    if (!fatfs_path || !buffer) {
        return NULL;
    }
    
    int drive = 0;
    const char* path_start = fatfs_path;
    /* Check if it's a FatFs-style path (e.g., "0:/", "1:/", etc.) */
    if (strlen(fatfs_path) >= 2 && fatfs_path[1] == ':') {
        /* Parse drive number */
        drive = fatfs_path[0] - '0';
        fatfs_path += 2;
    }
    if (drive >= 0 && drive < FF_VOLUMES) {
        /* Skip the '/' if present */
        if (*path_start == '/' || *path_start == '\\') {
            path_start++;
        }
        if (mounted_fs[drive].mounted == 1) {
            snprintf(buffer, buffer_size, "%s/%s", program_root[drive], path_start);
            FF_PRINTF("ff: %s -> %s\n", __func__, buffer);
            return buffer;
        }
    }
    return NULL;
}

/* Helper function to convert mode flags */
static const char* get_fopen_mode(uint8_t mode) {
    static char mode_str[4];
    memset(mode_str, 0, sizeof(mode_str));
    
    if (mode & FA_CREATE_NEW) {
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "w+b"); // TODO: does 'b' works on linux?
            } else {
                strcpy(mode_str, "wb");
            }
        }
    } else if (mode & FA_CREATE_ALWAYS) {
        if (mode & FA_READ) {
            strcpy(mode_str, "w+b");
        } else {
            strcpy(mode_str, "wb");
        }
    } else if (mode & FA_OPEN_APPEND) {
        if (mode & FA_READ) {
            strcpy(mode_str, "a+b");
        } else {
            strcpy(mode_str, "ab");
        }
    } else if (mode & FA_OPEN_ALWAYS) {
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "r+b");
            } else {
                strcpy(mode_str, "wb");
            }
        } else {
            strcpy(mode_str, "rb");
        }
    } else { /* FA_OPEN_EXISTING */
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "r+b");
            } else {
                strcpy(mode_str, "r+b");
            }
        } else {
            strcpy(mode_str, "rb");
        }
    }
    
    return mode_str;
}

/* Mount/Unmount functions */
FRESULT f_mount(FATFS* fs, const char* path, uint8_t opt) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fs && opt) {
        return FR_INVALID_PARAMETER;
    }
    
    /* Parse drive number from path */
    int drive = -1;
    if (path && strlen(path) >= 2 && path[1] == ':') {
        drive = path[0] - '0';
    }
    
    if (drive < 0 || drive >= FF_VOLUMES) {
        return FR_INVALID_DRIVE;
    }
    
    if (fs) {
        /* Mount the filesystem */
        char translated_path[256];
        translate_path(path, translated_path, sizeof(translated_path));
        
        strncpy(fs->path, translated_path, sizeof(fs->path) - 1);
        fs->path[sizeof(fs->path) - 1] = '\0';
        fs->mounted = 1;
        fs->drv = drive;
        
        mounted_fs[drive] = *fs;
    }
    
    return FR_OK;
}

FRESULT f_unmount(const char* path) {
    FF_PRINTF("ff: %s\n", __func__);
    /* Parse drive number from path */
    if (path && strlen(path) >= 2 && path[1] == ':') {
        int drive = path[0] - '0';
        if ((drive >= 0) && (drive < FF_VOLUMES)) {
            memset(&mounted_fs[drive], 0, sizeof(FATFS));
            return FR_OK;
        }
    }
    return FR_INVALID_DRIVE;
}

/* File operations */
FRESULT f_open(FIL* fp, const char* path, uint8_t mode) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !path) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
    
    memset(fp, 0, sizeof(FIL));
    strncpy(fp->fname, real_path, sizeof(fp->fname) - 1);
    
    /* Check if file exists for CREATE_NEW */
    if (mode & FA_CREATE_NEW) {
        struct stat st;
        if (stat(real_path, &st) == 0) {
            return FR_EXIST;
        }
    }
    
    const char* mode_str = get_fopen_mode(mode);
    fp->fp = fopen(real_path, mode_str);
    
    if (!fp->fp) {
        if (errno == ENOENT) {
            return FR_NO_FILE;
        } else if (errno == EACCES) {
            return FR_DENIED;
        }
        return FR_DISK_ERR;
    }
    
    /* Get file size */
    fseek(fp->fp, 0, SEEK_END);
    fp->fsize = ftell(fp->fp);
    fseek(fp->fp, 0, SEEK_SET);
    fp->fptr = 0;
    fp->flag = mode;
    
    return FR_OK;
}

FRESULT f_close(FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp) {
        return FR_INVALID_OBJECT;
    }
    
    if (fclose(fp->fp) != 0) {
        return FR_DISK_ERR;
    }
    
    fp->fp = NULL;
    return FR_OK;
}

FRESULT f_read(FIL* fp, void* buff, uint32_t btr, uint32_t* br) {
    // FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp || !buff) {
        return FR_INVALID_OBJECT;
    }
    
    size_t bytes_read = fread(buff, 1, btr, fp->fp);
    if (br) {
        *br = bytes_read;
    }
    
    fp->fptr += bytes_read;

    if (feof(fp->fp)) {
        FF_PRINTF("ff: f_read - EOF\n");
    }
    if (ferror(fp->fp)) {
        FF_PRINTF("ff: f_read - DISK_ERR\n");
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_write(FIL* fp, const void* buff, uint32_t btw, uint32_t* bw) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp || !buff) {
        return FR_INVALID_OBJECT;
    }
    
    if (!(fp->flag & FA_WRITE)) {
        return FR_DENIED;
    }
    
    size_t bytes_written = fwrite(buff, 1, btw, fp->fp);
    if (bw) {
        *bw = bytes_written;
    }
    
    fp->fptr += bytes_written;
    if (fp->fptr > fp->fsize) {
        fp->fsize = fp->fptr;
    }
    
    if (ferror(fp->fp)) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_lseek(FIL* fp, uint32_t ofs) {
    // FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp) {
        return FR_INVALID_OBJECT;
    }
    
    if (fseek(fp->fp, ofs, SEEK_SET) != 0) {
        return FR_DISK_ERR;
    }
    
    fp->fptr = ofs;
    return FR_OK;
}

uint32_t f_tell(FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp) {
        return 0;
    }
    return fp->fptr;
}

uint32_t f_size(FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp) {
        return 0;
    }
    return fp->fsize;
}

FRESULT f_sync(FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp) {
        return FR_INVALID_OBJECT;
    }
    
    if (fflush(fp->fp) != 0) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

// FRESULT f_truncate(FIL* fp) {
//     FF_PRINTF("ff: %s\n", __func__);
//     if (!fp || !fp->fp) {
//         return FR_INVALID_OBJECT;
//     }
//     if (truncate(fp->fname, fp->fptr) != 0) {
//         return FR_DISK_ERR;
//     }
//     fp->fsize = fp->fptr;
//     return FR_OK;
// }

/* Directory operations */
FRESULT f_opendir(DIR_WRAPPER* dp, const char* path) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!dp || !path) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
    
    dp->dir = opendir(real_path);
    if (!dp->dir) {
        if (errno == ENOENT) {
            return FR_NO_PATH;
        }
        return FR_DISK_ERR;
    }

    dp->find = NULL;
    
    strncpy(dp->path, real_path, sizeof(dp->path) - 1);
    dp->path[sizeof(dp->path) - 1] = '\0';
    dp->index = 0;
    
    return FR_OK;
}

FRESULT f_closedir(DIR_WRAPPER* dp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!dp || !dp->dir) {
        return FR_INVALID_OBJECT;
    }
    
    if (closedir((DIR*)dp->dir) != 0) {
        return FR_DISK_ERR;
    }
    
    dp->dir = NULL;
    return FR_OK;
}

FRESULT f_readdir(DIR_WRAPPER* dp, FILINFO* fno) {
    if (!dp || !dp->dir) {
        return FR_INVALID_OBJECT;
    }
    
    struct dirent* entry = readdir((DIR*)dp->dir);
    
    if (!entry) {
        if (fno) {
            fno->fname[0] = '\0';
        }
        return FR_OK;
    }
    
    if (fno) {
        strncpy(fno->fname, entry->d_name, sizeof(fno->fname) - 1);
        fno->fname[sizeof(fno->fname) - 1] = '\0';
        FF_PRINTF("ff: %s: %s\n", __func__, fno->fname);
        
        /* Get file stats */
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dp->path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            fno->fsize = st.st_size;
            fno->fattrib = 0;
            
            if (S_ISDIR(st.st_mode)) {
                fno->fattrib |= AM_DIR;
            }
            if (!(st.st_mode & S_IWUSR)) {
                fno->fattrib |= AM_RDO;
            }
            
            /* Convert time */
            struct tm* tm_info = localtime(&st.st_mtime);
            fno->fdate = ((tm_info->tm_year - 80) << 9) | 
                        ((tm_info->tm_mon + 1) << 5) | 
                        tm_info->tm_mday;
            fno->ftime = (tm_info->tm_hour << 11) | 
                        (tm_info->tm_min << 5) | 
                        (tm_info->tm_sec / 2);
        }
    }
    
    dp->index++;
    return FR_OK;
}

FRESULT f_mkdir(const char* path) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!path) {
        return FR_INVALID_PARAMETER;
    }

    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
#ifdef _WIN32
    if (mkdir(real_path) != 0) {
#else
    if (mkdir(real_path, 0755) != 0) {
#endif
        if (errno == EEXIST) {
            return FR_EXIST;
        } else if (errno == ENOENT) {
            return FR_NO_PATH;
        }
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_unlink(const char* path) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!path) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
    
    struct stat st;
    if (stat(real_path, &st) != 0) {
        return FR_NO_FILE;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (rmdir(real_path) != 0) {
            return FR_DENIED;
        }
    } else {
        if (unlink(real_path) != 0) {
            return FR_DENIED;
        }
    }
    
    return FR_OK;
}

FRESULT f_rename(const char* path_old, const char* path_new) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!path_old || !path_new) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_old[256], translated_new[256];
    const char* real_old = translate_path(path_old, translated_old, sizeof(translated_old));
    const char* real_new = translate_path(path_new, translated_new, sizeof(translated_new));
    
    if (!real_old || !real_new) {
        return FR_INVALID_NAME;
    }
    
    if (rename(real_old, real_new) != 0) {
        if (errno == ENOENT) {
            return FR_NO_FILE;
        } else if (errno == EEXIST) {
            return FR_EXIST;
        }
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_stat(const char* path, FILINFO* fno) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!path || !fno) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
    
    struct stat st;
    if (stat(real_path, &st) != 0) {
        if (errno == ENOENT) {
            return FR_NO_FILE;
        }
        return FR_DISK_ERR;
    }
    
    /* Extract filename from path */
    const char* fname = strrchr(real_path, '/');
    if (!fname) {
        fname = strrchr(real_path, '\\');
    }
    if (!fname) {
        fname = real_path;
    } else {
        fname++;
    }
    
    strncpy(fno->fname, fname, sizeof(fno->fname) - 1);
    fno->fname[sizeof(fno->fname) - 1] = '\0';
    fno->fsize = st.st_size;
    fno->fattrib = 0;
    
    if (S_ISDIR(st.st_mode)) {
        fno->fattrib |= AM_DIR;
    }
    if (!(st.st_mode & S_IWUSR)) {
        fno->fattrib |= AM_RDO;
    }
    
    /* Convert time */
    struct tm* tm_info = localtime(&st.st_mtime);
    fno->fdate = ((tm_info->tm_year - 80) << 9) | 
                ((tm_info->tm_mon + 1) << 5) | 
                tm_info->tm_mday;
    fno->ftime = (tm_info->tm_hour << 11) | 
                (tm_info->tm_min << 5) | 
                (tm_info->tm_sec / 2);
    
    return FR_OK;
}

/* String/Text operations */
FRESULT f_gets(char* buff, int len, FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!buff || !fp || !fp->fp) {
        return FR_INVALID_PARAMETER;
    }
    
    if (fgets(buff, len, fp->fp) == NULL) {
        if (feof(fp->fp)) {
            buff[0] = '\0';
            return FR_OK;
        }
        return FR_DISK_ERR;
    }
    
    fp->fptr = ftell(fp->fp);
    return FR_OK;
}

FRESULT f_puts(const char* str, FIL* fp) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!str || !fp || !fp->fp) {
        return FR_INVALID_PARAMETER;
    }
    
    if (fputs(str, fp->fp) == EOF) {
        return FR_DISK_ERR;
    }
    
    fp->fptr = ftell(fp->fp);
    if (fp->fptr > fp->fsize) {
        fp->fsize = fp->fptr;
    }
    
    return FR_OK;
}

FRESULT f_printf(FIL* fp, const char* fmt, ...) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!fp || !fp->fp || !fmt) {
        return FR_INVALID_PARAMETER;
    }
    
    va_list args;
    va_start(args, fmt);
    int ret = vfprintf(fp->fp, fmt, args);
    va_end(args);
    
    if (ret < 0) {
        return FR_DISK_ERR;
    }
    
    fp->fptr = ftell(fp->fp);
    if (fp->fptr > fp->fsize) {
        fp->fsize = fp->fptr;
    }
    
    return FR_OK;
}

/* Directory navigation */
FRESULT f_chdir(const char* path) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!path) {
        return FR_INVALID_PARAMETER;
    }
    
    char translated_path[256];
    const char* real_path = translate_path(path, translated_path, sizeof(translated_path));
    if (!real_path) {
        return FR_INVALID_NAME;
    }
    
    if (chdir(real_path) != 0) {
        if (errno == ENOENT) {
            return FR_NO_PATH;
        }
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

// FRESULT f_chdrive(const char* path) {
//     /* Parse drive number and update current volume */
//     if (path && strlen(path) >= 2 && path[1] == ':') {
//         int drive = path[0] - '0';
//         if (drive >= 0 && drive < FF_VOLUMES) {
//             /* Just accept the drive change */
//             return FR_OK;
//         }
//     }
//     return FR_INVALID_DRIVE;
// }

FRESULT f_getcwd(char* buff, uint32_t len) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!buff) {
        return FR_INVALID_PARAMETER;
    }
    
    if (getcwd(buff, len) == NULL) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

/* Pattern matching function (supports * and ? wildcards) */
static int pattern_match(const char* pattern, const char* string) {
    const char* s = string;
    const char* p = pattern;

    if (!*p) {
        return 0;
    }

    while (*s) {
        if ((*p == '?') || (toupper(*p) == toupper(*s))) {
            s++;
            p++;
        } else if (*p == '*') {
            SDL_assert(p[1] != '*');
            if  (*s == p[1]) {
                p++;
            } else {
                s++;
            }
        } else {
            return 0;
        }
    }
    
    while (*p == '*') {
        p++;
    }
    return !*p;
}

/* Find first file matching pattern */
FRESULT f_findfirst(DIR_WRAPPER* dp, FILINFO* fno, const char* path, const char* pattern) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!dp || !fno || !path || !pattern) {
        return FR_INVALID_PARAMETER;
    }
    
    FRESULT res = f_opendir(dp, path);
    if (res != FR_OK) {
        return res;
    }
    
    dp->find = pattern;
    
    /* Find first matching file */
    return f_findnext(dp, fno);
}

/* Find next file matching pattern */
FRESULT f_findnext(DIR_WRAPPER* dp, FILINFO* fno) {
    FF_PRINTF("ff: %s\n", __func__);
    if (!dp || !fno) {
        return FR_INVALID_PARAMETER;
    }
    
    FRESULT res;
    /* Read directory entries until we find a match */
    while (1) {
        res = f_readdir(dp, fno);
        if (res != FR_OK) {
            return res;
        }
        
        /* End of directory */
        if (fno->fname[0] == '\0') {
            return FR_OK;
        }
        
        /* Check if filename matches pattern */
        if (pattern_match(dp->find, fno->fname)) {
            return FR_OK;
        }
    }
}
