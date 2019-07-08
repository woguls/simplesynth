#ifndef LPF_H
#define LPF_H

/*
 * 24db resonant lowpass digital filter.
 *
 * Source: Stilson/Smith CCRMA paper
 * http://www.musicdsp.org/archive.php?classid=3#24
 */
class LowPassFilter {
public:
    LowPassFilter(float samplerate);
    ~LowPassFilter();
    void reset();
    void calcCoef();
    float process(float input);

    /*
     * Set the sample rate
     * @param fs new sample rate in Hz
     */
    void setSampleRate(float fs) {
        if (samplerate != fs) {
            samplerate = (fs > 0.0) ? fs : 44100.0f;
            calcCoef();
        }
    }

    /*
     * Get the current filter cutoff frequency (in Hz)
     */
    float getCutoff() {
        return cutoff;
    }

    /*
     * Set the filter cutoff frequency
     * @param c cutoff freq in Hz
     */
    void setCutoff(float c) {
        if (cutoff != c) {
            cutoff = c;
            calcCoef();
        }
    }

    /*
     * Get the current filter resonance value
     */
    float getResonance() {
        return resonance;
    }

    /*
     * Set the filter resonance value
     * @param r 0.0 .. 1.0
     */
    void setResonance(float r) {
        if (resonance != r) {
            resonance = r;
            calcCoef();
        }
    }
protected:
    float samplerate;  // sampling frequency in Hz (e.g. 44100Hz)
    float cutoff, resonance, rs, p, k;
    float x, y1, y2, y3, y4, oldx, oldy1, oldy2, oldy3;
};

#endif  // #ifndef LPF_H
