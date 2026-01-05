// vibecoded in Anthropic Claude Opus 4.1
#include "fatfs_wrapper.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

/* Static storage for mounted filesystems */
static FATFS mounted_fs[10];
static int mounted_count = 0;

/* Helper function to convert mode flags */
static const char* get_fopen_mode(uint8_t mode) {
    static char mode_str[4];
    memset(mode_str, 0, sizeof(mode_str));
    
    if (mode & FA_CREATE_NEW) {
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "w+");
            } else {
                strcpy(mode_str, "w");
            }
        }
    } else if (mode & FA_CREATE_ALWAYS) {
        if (mode & FA_READ) {
            strcpy(mode_str, "w+");
        } else {
            strcpy(mode_str, "w");
        }
    } else if (mode & FA_OPEN_APPEND) {
        if (mode & FA_READ) {
            strcpy(mode_str, "a+");
        } else {
            strcpy(mode_str, "a");
        }
    } else if (mode & FA_OPEN_ALWAYS) {
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "r+");
            } else {
                strcpy(mode_str, "w");
            }
        } else {
            strcpy(mode_str, "r");
        }
    } else { /* FA_OPEN_EXISTING */
        if (mode & FA_WRITE) {
            if (mode & FA_READ) {
                strcpy(mode_str, "r+");
            } else {
                strcpy(mode_str, "r+");
            }
        } else {
            strcpy(mode_str, "r");
        }
    }
    
    return mode_str;
}

/* Mount/Unmount functions */
FRESULT f_mount(FATFS* fs, const char* path, uint8_t opt) {
    if (!fs && opt) {
        return FR_INVALID_PARAMETER;
    }
    
    if (fs) {
        strncpy(fs->path, path, sizeof(fs->path) - 1);
        fs->mounted = 1;
        fs->drv = mounted_count;
        
        if (mounted_count < 10) {
            mounted_fs[mounted_count++] = *fs;
        }
    }
    
    return FR_OK;
}

FRESULT f_unmount(const char* path) {
    /* Find and remove mounted filesystem */
    for (int i = 0; i < mounted_count; i++) {
        if (strcmp(mounted_fs[i].path, path) == 0) {
            mounted_fs[i].mounted = 0;
            return FR_OK;
        }
    }
    return FR_INVALID_DRIVE;
}

/* File operations */
FRESULT f_open(FIL* fp, const char* path, uint8_t mode) {
    if (!fp || !path) {
        return FR_INVALID_PARAMETER;
    }
    
    memset(fp, 0, sizeof(FIL));
    strncpy(fp->fname, path, sizeof(fp->fname) - 1);
    
    /* Check if file exists for CREATE_NEW */
    if (mode & FA_CREATE_NEW) {
        struct stat st;
        if (stat(path, &st) == 0) {
            return FR_EXIST;
        }
    }
    
    const char* mode_str = get_fopen_mode(mode);
    fp->fp = fopen(path, mode_str);
    
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
    if (!fp || !fp->fp || !buff) {
        return FR_INVALID_OBJECT;
    }
    
    size_t bytes_read = fread(buff, 1, btr, fp->fp);
    if (br) {
        *br = bytes_read;
    }
    
    fp->fptr += bytes_read;
    
    if (ferror(fp->fp)) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_write(FIL* fp, const void* buff, uint32_t btw, uint32_t* bw) {
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
    if (!fp || !fp->fp) {
        return 0;
    }
    return fp->fptr;
}

uint32_t f_size(FIL* fp) {
    if (!fp) {
        return 0;
    }
    return fp->fsize;
}

FRESULT f_sync(FIL* fp) {
    if (!fp || !fp->fp) {
        return FR_INVALID_OBJECT;
    }
    
    if (fflush(fp->fp) != 0) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_truncate(FIL* fp) {
    if (!fp || !fp->fp) {
        return FR_INVALID_OBJECT;
    }
    
    if (ftruncate(fileno(fp->fp), fp->fptr) != 0) {
        return FR_DISK_ERR;
    }
    
    fp->fsize = fp->fptr;
    return FR_OK;
}

/* Directory operations */
FRESULT f_opendir(DIR_WRAPPER* dp, const char* path) {
    if (!dp || !path) {
        return FR_INVALID_PARAMETER;
    }
    
    dp->dir = opendir(path);
    if (!dp->dir) {
        if (errno == ENOENT) {
            return FR_NO_PATH;
        }
        return FR_DISK_ERR;
    }
    
    strncpy(dp->path, path, sizeof(dp->path) - 1);
    dp->index = 0;
    
    return FR_OK;
}

FRESULT f_closedir(DIR_WRAPPER* dp) {
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
    if (!path) {
        return FR_INVALID_PARAMETER;
    }
#ifdef _WIN32
    if (mkdir(path) != 0) {
#else
    if (mkdir(path, 0755) != 0) {
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
    if (!path) {
        return FR_INVALID_PARAMETER;
    }
    
    struct stat st;
    if (stat(path, &st) != 0) {
        return FR_NO_FILE;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (rmdir(path) != 0) {
            return FR_DENIED;
        }
    } else {
        if (unlink(path) != 0) {
            return FR_DENIED;
        }
    }
    
    return FR_OK;
}

FRESULT f_rename(const char* path_old, const char* path_new) {
    if (!path_old || !path_new) {
        return FR_INVALID_PARAMETER;
    }
    
    if (rename(path_old, path_new) != 0) {
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
    if (!path || !fno) {
        return FR_INVALID_PARAMETER;
    }
    
    struct stat st;
    if (stat(path, &st) != 0) {
        if (errno == ENOENT) {
            return FR_NO_FILE;
        }
        return FR_DISK_ERR;
    }
    
    /* Extract filename from path */
    const char* fname = strrchr(path, '/');
    if (!fname) {
        fname = strrchr(path, '\\');
    }
    if (!fname) {
        fname = path;
    } else {
        fname++;
    }
    
    strncpy(fno->fname, fname, sizeof(fno->fname) - 1);
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
    if (!path) {
        return FR_INVALID_PARAMETER;
    }
    
    if (chdir(path) != 0) {
        if (errno == ENOENT) {
            return FR_NO_PATH;
        }
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}

FRESULT f_chdrive(const char* path) {
    /* Not applicable on Unix-like systems */
    /* Just return OK to maintain compatibility */
    (void)path;
    return FR_OK;
}

FRESULT f_getcwd(char* buff, uint32_t len) {
    if (!buff) {
        return FR_INVALID_PARAMETER;
    }
    
    if (getcwd(buff, len) == NULL) {
        return FR_DISK_ERR;
    }
    
    return FR_OK;
}
