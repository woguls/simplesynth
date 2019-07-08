/*
 * Simple Synth audio efffect based on DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2018 Christopher Arndt <info@chrisarndt.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>

#include "PluginSimpleSynth.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

PluginSimpleSynth::PluginSimpleSynth()
    : Plugin(paramCount, 1, 0)  // paramCount params, 1 program(s), 0 states
{
    osc1 = sawOsc();
    ampenv = new ADSR();
    fenv = new ADSR();
    lpf = new LowPassFilter(getSampleRate());
}

PluginSimpleSynth::~PluginSimpleSynth() {
    delete ampenv;
    delete fenv;
    delete osc1;
    delete lpf;
}

// -----------------------------------------------------------------------
// Init

void PluginSimpleSynth::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;

    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.1f;
    parameter.hints = kParameterIsAutomable;

    switch (index) {
        case paramVolume:
            parameter.name = "Volume";
            parameter.symbol = "volume";
            //parameter.hints |= kParameterIsLogarithmic;
            break;
        case paramAmpEnvAttack:
            parameter.name = "Amp. Env. Attack";
            parameter.symbol = "aenv_attack";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            break;
        case paramAmpEnvDecay:
            parameter.name = "Amp. Env. Decay";
            parameter.symbol = "aenv_decay";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 10.0f;
            break;
        case paramAmpEnvSustain:
            parameter.name = "Amp. Env. Sustain";
            parameter.symbol = "aenv_sustain";
            parameter.ranges.def = 1.0f;
            break;
        case paramAmpEnvRelease:
            parameter.name = "Amp. Env. Release";
            parameter.symbol = "aenv_release";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            break;
        case paramFEnvAttack:
            parameter.name = "F. Env. Attack";
            parameter.symbol = "fenv_attack";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            break;
        case paramFEnvDecay:
            parameter.name = "F. Env. Decay";
            parameter.symbol = "fenv_decay";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 10.0f;
            break;
        case paramFEnvSustain:
            parameter.name = "F. Env. Sustain";
            parameter.symbol = "fenv_sustain";
            parameter.ranges.def = 1.0f;
            break;
        case paramFEnvRelease:
            parameter.name = "F. Env. Release";
            parameter.symbol = "fenv_release";
            parameter.ranges.min = 0.001f;
            parameter.ranges.max = 10.0f;
            break;
        case paramLPFCutoff:
            parameter.name = "Cutoff";
            parameter.symbol = "lpf_cutoff";
            parameter.ranges.min = 16.0f;
            parameter.ranges.max = 20000.0f;
            parameter.ranges.def = 20000.0f;
            parameter.ranges.def = 1.0f;
            break;
        case paramLPFResonance:
            parameter.name = "Resonance";
            parameter.symbol = "lpf_reso";
            parameter.ranges.def = 0.0f;
            break;
        case paramLPFEnvAmount:
            parameter.name = "F. Env.->LPF";
            parameter.symbol = "lpf_fenv_amount";
            parameter.ranges.def = 0.0f;
            break;
        }
}

/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void PluginSimpleSynth::initProgramName(uint32_t index, String& programName) {
    switch (index) {
        case 0:
            programName = "Default";
            break;
    }
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void PluginSimpleSynth::sampleRateChanged(double newSampleRate) {
    fSampleRate = newSampleRate;
    lpf->setSampleRate(newSampleRate);
}

/**
  Get the current value of a parameter.
*/
float PluginSimpleSynth::getParameterValue(uint32_t index) const {
    return fParams[index];
}

/**
  Change a parameter value.
*/
void PluginSimpleSynth::setParameterValue(uint32_t index, float value) {
    fParams[index] = value;

    switch (index) {
        case paramVolume:
            // do something when volume param is set
            break;
        case paramAmpEnvAttack:
            ampenv->setAttackRate(value * fSampleRate);
            break;
        case paramAmpEnvDecay:
            ampenv->setDecayRate(value * fSampleRate);
            break;
        case paramAmpEnvSustain:
            ampenv->setSustainLevel(value);
            break;
        case paramAmpEnvRelease:
            ampenv->setReleaseRate(value * fSampleRate);
            break;
        case paramFEnvAttack:
            fenv->setAttackRate(value * fSampleRate);
            break;
        case paramFEnvDecay:
            fenv->setDecayRate(value * fSampleRate);
            break;
        case paramFEnvSustain:
            fenv->setSustainLevel(value);
            break;
        case paramFEnvRelease:
            fenv->setReleaseRate(value * fSampleRate);
            break;
        case paramLPFCutoff:
            // nothing to do
            break;
        case paramLPFResonance:
            lpf->setResonance(value);
            break;
        case paramLPFEnvAmount:
            // nothing to do
            break;
        }
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void PluginSimpleSynth::loadProgram(uint32_t index) {
    switch (index) {
        case 0:
            setParameterValue(paramVolume, 0.01f);
            setParameterValue(paramAmpEnvAttack, 0.1f);
            setParameterValue(paramAmpEnvDecay, 0.3f);
            setParameterValue(paramAmpEnvSustain, 0.8f);
            setParameterValue(paramAmpEnvRelease, 0.2f);
            setParameterValue(paramFEnvAttack, 0.1f);
            setParameterValue(paramFEnvDecay, 0.0f);
            setParameterValue(paramFEnvSustain, 1.0f);
            setParameterValue(paramFEnvRelease, 0.1f);
            setParameterValue(paramLPFCutoff, 20000.0f);
            setParameterValue(paramLPFResonance, 0.0f);
            break;
    }
}

// -----------------------------------------------------------------------
// Process

void PluginSimpleSynth::activate() {
    for (int i=0; i < 128; i++) {
        noteState[i] = false;
    }

    sampleRateChanged(getSampleRate());
    ampenv->reset();
    fenv->reset();
    lpf->reset();
    osc1->SetFrequency(440.0f / fSampleRate);
    loadProgram(0);
}

void PluginSimpleSynth::run(const float**, float** outputs, uint32_t frames,
                            const MidiEvent *midiEvents, uint32_t midiEventCount) {
    // get the left and right audio outputs
    float* const outL = outputs[0];
    float* const outR = outputs[1];

    uint8_t note, velo;
    float freq, vol = fParams[paramVolume];

    for (uint32_t count, pos=0, curEventIndex=0; pos<frames;) {
        for (;curEventIndex < midiEventCount && pos >= midiEvents[curEventIndex].frame; ++curEventIndex) {
            if (midiEvents[curEventIndex].size > MidiEvent::kDataSize)
                continue;

            const uint8_t* data = midiEvents[curEventIndex].data;
            const uint8_t status = data[0] & 0xF0;

            switch (status) {
                case 0x90:
                    note = data[1];
                    velo = data[2];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (noteState[note]) {
                        if (velo == 0) {
                            noteState[note] = false;
                            ampenv->gate(false);
                        }
                    }
                    else if (velo > 0) {
                        freq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
                        osc1->SetFrequency(freq / fSampleRate);
                        noteState[note] = true;
                        ampenv->gate(true);
                        break;
                    }
                    break;
                case 0x80:
                    note = data[1];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (noteState[note]) {
                        noteState[note] = false;
                        ampenv->gate(false);
                    }
                    break;
            }
        }

        if (curEventIndex < midiEventCount && midiEvents[curEventIndex].frame < frames)
            count = midiEvents[curEventIndex].frame - pos;
        else
            count = frames - pos;

        for (uint32_t i=0; i<count; ++i) {
            lpf->setCutoff(fParams[paramLPFCutoff] * fenv->process());
            float sample = lpf->process(osc1->Process()) * ampenv->process() * vol;
            outL[pos + i] = sample;
            outR[pos + i] = sample;
        }

        pos += count;
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new PluginSimpleSynth();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
