#ifndef __SYNTH_H
#define __SYNTH_H

#include "midi.h"
#include "scope_xy.h"
extern ScopeXY scope;

void synthInit();
void synthHandleMidi(MidiMessageT m);

#endif // __SYNTH_H