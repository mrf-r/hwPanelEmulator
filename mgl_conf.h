#ifndef __MGL_CONF_H
#define __MGL_CONF_H

#include <SDL.h>
#define MGL_ASSERT SDL_assert

// #include "config.h"
// #define MGL_SINGLEDISPLAY

#define COLOR_OFF ((MglColor){.wrd=0x0})
#define COLOR_ON ((MglColor){.wrd=0x1})
#define COLOR_INVERT ((MglColor){.wrd=0x1000})

// #define DISPLAY_SIZE_X FORM_SIZE_X
// #define DISPLAY_SIZE_Y FORM_SIZE_Y

#endif // __MGL_CONF_H