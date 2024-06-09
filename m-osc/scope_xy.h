#ifndef _SCOPE_XY_H
#define _SCOPE_XY_H

#include <stdint.h>
#include "mgl.h"

#define LCG_A 1103515245
#define LCG_B 12345
#define SCOPE_FADE_COEF 128 // less is faster
#define SCOPE_X 512

typedef struct
{
    int16_t buf_x;
    int16_t buf_y;
    uint32_t lcg;
    int16_t buf[SCOPE_X];
} ScopeXY;

static inline void scopeXYsr(
    ScopeXY* const s,
    const int16_t x,
    const int16_t y)
{
    s->buf_x = x;
    s->buf_y = y;
    unsigned pos_x = SCOPE_X * (x + 0x8000) / 65536;
    s->buf[pos_x] = y;
}

static inline void scopeXYframe(
    ScopeXY* const s,
    const uint16_t x,
    const uint16_t y,
    const uint16_t xs,
    const uint16_t ys,
    const MglColor on,
    const MglColor off)
{
    // for (int i = 0; i < ((xs * ys / SCOPE_FADE_COEF) + 1); i++) {
    //     uint32_t lcg = s->lcg * LCG_A + LCG_B;
    //     uint32_t lcg2 = lcg * LCG_A + LCG_B;
    //     s->lcg = lcg2;
    //     uint16_t px = ((lcg / 65536) * xs) / 65536;
    //     uint16_t py = ((lcg / 65536) * ys) / 65536;
    //     mgsWorkingArea(x + px, y + py, 1, 1);
    //     mgdFill(off);
    // }
    // uint32_t sx = ((int32_t)s->buf_x + 0x8000) * xs / 0x10000;
    // uint32_t sy = ((int32_t)s->buf_y + 0x8000) * ys / 0x10000;
    // mgsWorkingArea(x + sx, y + sy, 1, 1);
    // mgdFill(on);

    mgsWorkingArea(x, y, xs, ys);
    mgdFill(off);
    for (unsigned i = 0; i < SCOPE_X; i++) {
        uint16_t px = (uint32_t)i * xs / SCOPE_X;
        uint16_t py = ((int32_t)(s->buf[i]) + 0x8000) * ys / 65536;
        mgsWorkingArea(x + px, y + py, 1, 1);
        mgdFill(on);
    }
}

#endif // _SCOPE_XY_H