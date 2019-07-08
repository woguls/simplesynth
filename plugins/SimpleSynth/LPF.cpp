#include <math.h>

#include "LPF.hpp"

LowPassFilter::LowPassFilter(float fs)
    : samplerate(fs)
{
    reset();
}

LowPassFilter::~LowPassFilter(void) {
}

void LowPassFilter::reset() {
    y1=y2=y3=y4=oldx=oldy1=oldy2=oldy3=0;
}

/* Calculate filter coefficients
 */
void LowPassFilter::calcCoef() {
    float f = (cutoff + cutoff) / samplerate;  // 0.0 .. 1.0
    p = f * (1.8f - 0.8f * f);
    k = p + p - 1.f;
    float t = (1.f - p) * 1.386249f;
    float t2 = 12.f + t * t;
    rs = resonance * (t2 + 6.f * t) / (t2 - 6.f * t);  // scale resonance
}

float LowPassFilter::process(float input) {
    // Inverted feedback for corner peaking
    x = input - rs * y4;

    // Four cascaded one-pole filters (bi-linear transform)
    y1 = x  * p + oldx  * p - k * y1;
    y2 = y1 * p + oldy1 * p - k * y2;
    y3 = y2 * p + oldy2 * p - k * y3;
    y4 = y3 * p + oldy3 * p - k * y4;

    // Clipper: band limited sigmoid
    y4 = y4 - (y4 * y4 * y4) / 6.f;

    oldx  = x;
    oldy1 = y1;
    oldy2 = y2;
    oldy3 = y3;
    return y4;
}
