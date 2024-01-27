#ifndef __MGL_CONF_H
#define __MGL_CONF_H

#define DEBUG

#ifdef DEBUG
#include <assert.h>
#define MGL_ASSERT assert
#else
#define MGL_ASSERT(...)
#endif

// #include "config.h"
// #define MGL_SINGLEDISPLAY

// #define DISPLAY_SIZE_X FORM_SIZE_X
// #define DISPLAY_SIZE_Y FORM_SIZE_Y

#endif // __MGL_CONF_H