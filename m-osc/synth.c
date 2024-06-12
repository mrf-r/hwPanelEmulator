#include <SDL.h>
#include "synth.h"
#include "vco.h"
#include "sinc.h"

#define SYNTH_ASSERT SDL_assert
#define SAMPLE_RATE_MULT 4

ScopeXY scope;
static SincFilter filter;
static VcoCore vco;
static VcoControlCv control_cv;
static uint8_t gate;

typedef struct {
    const char* const name;
    uint16_t* parameter;
} SynthParameter;

const SynthParameter parameter_table[] = {
    { "O1Pitch", &control_cv.ctrl_pitch },
    { "O1Octave", &control_cv.ctrl_octave },
    { "O1Amp", &control_cv.ctrl_mix },
    { "O2Phase", &control_cv.ctrl_phase },
    { "O2Pitch", &control_cv.ctrl_2pitch },
    { "O2Sync", &control_cv.ctrl_sync },
};

#define SYNTH_FIRST_CC 0x10
#define SYNTH_PARAMETERS_COUNT (sizeof(parameter_table) / sizeof(SynthParameter))

void synthHandleMidi(MidiMessageT m)
{
    if (MIDI_CIN_CONTROLCHANGE == m.cin) {
        // synth engine
        if ((m.byte2 >= SYNTH_FIRST_CC) && (m.byte2 < (SYNTH_FIRST_CC + SYNTH_PARAMETERS_COUNT))) {
            // msb
            // we need to request value from synth engine
            uint16_t new_msb = m.byte3 << 7;
            uint16_t* parameter = parameter_table[m.byte2 - SYNTH_FIRST_CC].parameter;
            SYNTH_ASSERT(parameter);
            uint16_t synth_msb = *parameter & 0x3F80;
            if (new_msb > synth_msb) {
                *parameter = new_msb;
            } else if (new_msb < synth_msb) {
                *parameter = new_msb | 0x7F;
            }
        } else if ((m.byte2 >= (SYNTH_FIRST_CC + 0x20)) && (m.byte2 < (SYNTH_FIRST_CC + 0x20 + SYNTH_PARAMETERS_COUNT))) {
            // lsb
            uint16_t* parameter = parameter_table[m.byte2 - SYNTH_FIRST_CC - 0x20].parameter;
            SYNTH_ASSERT(parameter);
            *parameter = (*parameter & 0x3F80) | m.byte3;
        }
    } else if (MIDI_CIN_NOTEON == m.cin) {
        if (0 == m.byte2) {
            // synth.calibration_request = 1;
        } else {
            if (gate)
                gate = 0;
            else
                gate = 1;
            // TODO: noteon? calibration test over parameter as well
            // gate = 1;
        }
    } else if (MIDI_CIN_NOTEOFF == m.cin) {
        // gate = 0;
    }
}

void synthInit()
{
    sincInitCoefficients(&filter, 1.f / ((float)SAMPLE_RATE_MULT * 2.f * 1.25f));
}

static inline int16_t audioProcessOneSample(VcoControlBlock* const vcb)
{
    float ret;
    for (unsigned i = 0; i < SAMPLE_RATE_MULT; i++) {
        // do whatever you like
        int16_t sample = vcoProcessSample(vcb, &vco);
        int16_t scope_pos = vco.out_phase;
        scopeXYsr(&scope, scope_pos, sample);
        ret = sincProcessSample(&filter, (float)sample * 0.95f);
    }
    return (int16_t)ret;
}

void synthAudioCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length)
{
    int16_t* __restrict in = buffer_in;
    int16_t* __restrict out = buffer_out;
    const int16_t* out_end = buffer_out + (length * 2); // stereo
    VcoControlBlock vcb;

    vcoProcessBlock(&control_cv, &vco, &vcb);
    // heavily inspired by the KORG Logue SDK https://github.com/korginc/logue-sdk
    for (; out_end > out; out += 2, in += 2) {
        int16_t result = audioProcessOneSample(&vcb);
        result *= gate;
        out[0] = result;
        out[1] = result;
        // out[0] = in[0];
        // out[1] = in[1];
    }
}
