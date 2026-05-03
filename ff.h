#ifndef FF_H
#define FF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fatfs_wrapper.h"

typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef unsigned char	BYTE;	/* char must be 8-bit */
typedef uint16_t		WORD;	/* 16-bit unsigned integer */
typedef uint32_t		DWORD;	/* 32-bit unsigned integer */
typedef uint64_t		QWORD;	/* 64-bit unsigned integer */
typedef WORD			WCHAR;	/* UTF-16 character type */

typedef DIR_WRAPPER     DIR;

#ifdef __cplusplus
}
#endif


#endif /* FF_H */
