//
//  WaveUtils.h
//
//  Created by Nigel Redmon on 2/18/13.
//
//

#ifndef WaveUtils_h
#define WaveUtils_h

#include "WaveTableOsc.hpp"

void fillTables(WaveTableOsc *osc, double *freqWaveRe, double *freqWaveIm, int numSamples);

// examples
WaveTableOsc *sawOsc(void);
WaveTableOsc *waveOsc(double *waveSamples, int tableLen);

#endif
