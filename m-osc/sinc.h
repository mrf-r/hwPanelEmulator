#ifndef _SINC_H
#define _SINC_H

// high sampling rate is fun!
// and brute force confolution as well)

#include <math.h>

#define ONLY_POW2_TAPS
#define SINC_TAPS 32

static const float PI = 3.141592653589793f;

typedef struct
{
    float taps[SINC_TAPS];
    float coefficients[SINC_TAPS];
    unsigned position;
} SincFilter;

// cutoff from 0.01 to 0.49
void sincInitCoefficients(SincFilter* const f, const float cutoff)
{
    float offset = SINC_TAPS / 2.f - 0.5f;
    float w = cutoff * 2.0f * PI;
    for (unsigned i = 0; i < SINC_TAPS; i++) {
        float x = fabs((float)i - offset);
        float vc = 2.0f * cutoff;
        if (x >= 0.49f) {
            vc *= sinf(x * w) / (x * w);
        }
        f->coefficients[i] = vc;
    }
    // init
    for (unsigned i = 0; i < SINC_TAPS; i++) {
        f->taps[i] = 0;
    }
    f->position = 0;
}

float sincProcessSample(SincFilter* const f, const float in)
{
    f->taps[f->position] = in;
#ifdef ONLY_POW2_TAPS
    f->position = (f->position + 1) & (SINC_TAPS - 1);
#else
    f->position++;
    if (f->position >= SINC_TAPS) {
        f->position = 0;
    }
#endif
    float result = 0.f;
    for (unsigned i = 0; i < SINC_TAPS; i++) {
#ifdef ONLY_POW2_TAPS
        unsigned taps_pos = (f->position + i) & (SINC_TAPS - 1);
#else
        unsigned taps_pos = f->position + i;
        if (taps_pos >= SINC_TAPS)
            taps_pos -= SINC_TAPS;
#endif
        float r = f->coefficients[i] * f->taps[taps_pos];
        result += r;
    }
    return result;
}

#endif // _SINC_H