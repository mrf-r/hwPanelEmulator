#ifndef _VCO_H
#define _VCO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "scale.h"

#define PITCH_OCTAVES_RANGE 5
#define CALIB_OCTAVES_DISTANCE 2
#define CALIB_OCTAVES_OFFSET 1
#define GEN1_OCTAVES 4
#define GEN2_MAX_OCTAVE_OFFSET 6
#define GEN2_MAX_PHASE_SPEED_HZ 768

typedef struct {
    uint16_t ctrl_pitch;
    uint16_t ctrl_octave;
    uint16_t ctrl_mix;
    uint16_t ctrl_phase;
    uint16_t ctrl_2pitch;
    uint16_t ctrl_sync;
} VcoControlCv;

typedef struct {
    int32_t lcg;
    int32_t gen1;
    int32_t gen2;
    int32_t phase2_acc;
    int32_t glide;
    int16_t out_phase;
    int16_t out_debug;
} VcoCore;

typedef struct {
    int32_t base_inc;
    int32_t base_recp;

    uint32_t o1_p1mul;
    uint32_t o1_p2mul;
    uint32_t o1_p1inc;
    uint32_t o1_p2inc;
    int32_t o1_p1amp;
    int32_t o1_p2amp;

    uint32_t o2_phase_inc;
    uint32_t o2_inc;
    uint32_t o2_sync;
    int32_t o1_amp;
    int32_t o2_amp;
} VcoControlBlock;

#define CELL_STEPS (65536 / SCALE_TABLE_SIZE)
static inline void oscIncGet(uint16_t pitch, int32_t* inc, int32_t* recp)
{
    uint32_t pos = pitch / CELL_STEPS;
    uint32_t spos = pitch & (CELL_STEPS - 1);
    uint32_t v0 = table_pitch_inc[pos].inc;
    *recp = table_pitch_inc[pos].recp;
    int32_t diff = table_pitch_inc[pos + 1].inc - v0;
    *inc = v0 + diff * spos / CELL_STEPS;
}

static inline int32_t pd(uint32_t inc, int32_t noise16)
{
    return (inc / 32768) * noise16;
}

// fast approximation of exp2(x/4096)
static inline uint16_t exp2_fast(const uint16_t x)
{
    uint32_t e = x / 4096;
    uint32_t s = x & (4096 - 1);
    e = 1 << e;
    s = s * e / 4096;
    e = e + s;
    return (uint16_t)e;
}

static inline int16_t vcoProcessSample(
    VcoControlBlock* const ctrl,
    VcoCore* const vco)
{
    int32_t lcg = vco->lcg;
    lcg = lcg * 1103515245 + 12345;
    vco->lcg = lcg;
    int32_t lcg16 = lcg / 65536;
    int32_t gen1new = vco->gen1 + ctrl->base_inc;
    vco->out_phase = gen1new / 65536;
    int32_t gen1p1 = gen1new * ctrl->o1_p1mul + pd(ctrl->o1_p1inc, lcg16);
    int32_t gen1p2 = gen1new * ctrl->o1_p2mul + pd(ctrl->o1_p2inc, lcg16);
    // x-fade
    int32_t gen1o = gen1p1 / (65536 / GEN1_OCTAVES) * ctrl->o1_p1amp
        + gen1p2 / (65536 / GEN1_OCTAVES) * ctrl->o1_p2amp;
    gen1o /= 65536;
    vco->out_debug = gen1o;
    // GEN2 is randomly hard synced to gen1core
    int32_t gen2new;
    if ((gen1new < vco->gen1) && (ctrl->o2_sync < (uint32_t)lcg16)) {
        // sync gen2
        uint32_t subpos_norm = (gen1new & 0x7FFFFFFF) * ctrl->base_recp;
        gen2new = (subpos_norm / 65536) * ctrl->o2_inc / 65536;
        // random phase on a rare events
        gen2new += lcg16 * ctrl->o2_sync;
    } else {
        gen2new = vco->gen2 + ctrl->o2_inc;
    }
    vco->phase2_acc -= ctrl->o2_phase_inc;
    int32_t gen2o = gen2new + pd(gen2new - vco->gen2, lcg16) + vco->phase2_acc;
    gen2o /= 65536;
    vco->gen1 = gen1new; // we need to keep previous value of vco->gen1 until now
    vco->gen2 = gen2new;
    int32_t mix = gen1o * ctrl->o1_amp + gen2o * ctrl->o2_amp;
    return mix / 65536;
}

static inline void vcoProcessBlock(
    VcoControlCv* const cv,
    VcoCore* const vco,
    VcoControlBlock* const ctrl
    // int16_t* const buffer_out,
    // const unsigned block_size
)
{
    uint16_t pitch = cv->ctrl_pitch << 2;
    const uint16_t octave = cv->ctrl_octave << 2;
    const uint16_t pitch2 = cv->ctrl_2pitch << 2;
    const uint16_t sync = cv->ctrl_sync << 2;
    const uint16_t mix = cv->ctrl_mix << 2;
    const uint16_t phase2 = cv->ctrl_phase << 2;

    // bonus glide
    const uint32_t k = 430 * 128; // limits max time
    uint32_t arg = 65535 - k * phase2 / 65536;
    // setting w is basically the SR scaling: w = freq / SR
    // here we can just shift b
    int32_t glidew = exp2_fast(arg);
    int32_t pitch_goal = pitch * 32768;
    vco->glide += (pitch_goal - vco->glide) / 65536 * glidew;
    pitch = vco->glide / 32768;

    oscIncGet(pitch, &ctrl->base_inc, &ctrl->base_recp);

    const uint32_t oct_fade_steps = 65536 / GEN1_OCTAVES; // 16384
    ctrl->o1_p1mul = 1 << (octave / oct_fade_steps);
    ctrl->o1_p2mul = ctrl->o1_p1mul * 2;
    ctrl->o1_p1inc = ctrl->base_inc * ctrl->o1_p1mul;
    ctrl->o1_p2inc = ctrl->base_inc * ctrl->o1_p2mul;
    ctrl->o1_p2amp = octave & (oct_fade_steps - 1);
    ctrl->o1_p1amp = oct_fade_steps - 1 - ctrl->o1_p2amp;

    uint16_t p2 = (uint32_t)pitch2 * pitch2 / 65536;
    ctrl->o2_inc = ctrl->base_inc + (ctrl->base_inc / (65536 / GEN2_MAX_OCTAVE_OFFSET)) * p2;
    ctrl->o2_sync = sync * sync / 65536;

    const uint32_t max_phase_inc = (uint32_t)(FCF * GEN2_MAX_PHASE_SPEED_HZ + 0.5);
    ctrl->o2_phase_inc = (max_phase_inc) / 65536 * exp2_fast(phase2);

    ctrl->o2_amp = -mix;
    ctrl->o1_amp = 65536 - 1 - mix;

    // int16_t* __restrict out = buffer_out;
    // const int16_t* out_end = buffer_out + block_size;
    // for (; out_end > out; out += 1) {
    //     out = vcoProcessSample(&ctrl, vco);
    // }
}

#ifdef __cplusplus
}
#endif

#endif // _VCO_H