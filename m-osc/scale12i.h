#ifndef _SCALE12I_H
#define _SCALE12I_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE 48000 // TODO: should be defined not here
#define SRCF_OFFSET 10
#define SRCF (4294967296.0 / ((double)SAMPLE_RATE) * (double)(1 << SRCF_OFFSET))

// 0..3FFF is equal to 0..128 semitones (128 shift)

static inline void scale12EdoGetFreqHz(const int32_t pitch, int32_t* const inc,
                                       int32_t* const spd) {
  static const int32_t table2[25] = {
      (int32_t)(SRCF * 8.175798915643707),
      (int32_t)(SRCF * 8.415368110219507),
      (int32_t)(SRCF * 8.661957218027252),
      (int32_t)(SRCF * 8.915771938225692),
      (int32_t)(SRCF * 9.177023997418988),
      (int32_t)(SRCF * 9.445931326274362),
      (int32_t)(SRCF * 9.722718241315029),
      (int32_t)(SRCF * 10.007615632040041),
      (int32_t)(SRCF * 10.300861153527183),
      (int32_t)(SRCF * 10.602699424679592),
      (int32_t)(SRCF * 10.913382232281373),
      (int32_t)(SRCF * 11.233168741032559),
      (int32_t)(SRCF * 11.562325709738575),
      (int32_t)(SRCF * 11.90112771383447),
      (int32_t)(SRCF * 12.249857374429663),
      (int32_t)(SRCF * 12.60880559406423),
      (int32_t)(SRCF * 12.978271799373287),
      (int32_t)(SRCF * 13.358564190862078),
      (int32_t)(SRCF * 13.75),
      (int32_t)(SRCF * 14.15290575384802),
      (int32_t)(SRCF * 14.567617547440307),
      (int32_t)(SRCF * 14.994481324147293),
      (int32_t)(SRCF * 15.433853164253883),
      (int32_t)(SRCF * 15.886099581993749),
      (int32_t)(SRCF * 16.351597831287414)  // KEEP OFFSET IN MIND!
  };
  // 128 semitones = 10.6 octaves
  // const int32_t offset_neg = 48;
  int32_t octave = ((pitch + (48 * 12 * 128)) / 128) * (65536 / 12 + 1) / 65536 - 48;
  int32_t fpos = pitch & 0x3F; // 2 elements per semitone
  int32_t cpos = (pitch - (octave * 12 * 128)) / 64; // 2 elements per semitone
  int32_t s1 = table2[cpos];
  int32_t s2 = table2[cpos + 1];
  int32_t res = s1 + ((s2 - s1) / 64) * fpos;
  if (octave > SRCF_OFFSET) {
    // too high frequency
    int32_t shift = octave - SRCF_OFFSET;
    *inc = res * (1 << shift);
    *spd = (res / 65536) * (1 << shift);
  } else {
    // normal frequency - table divide
    int32_t shift = SRCF_OFFSET - octave;
    res = res / (1 << shift);
    *inc = res;
    *spd = res / 65536;
  }
}

#undef SAMPLE_RATE
#undef SRCF_OFFSET
#undef SRCF

#ifdef __cplusplus
}
#endif

#endif  // _SCALE12I_H