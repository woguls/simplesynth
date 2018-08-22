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
    sampleRateChanged(getSampleRate());
    osc = sawOsc();
    env = new ADSR();
    loadProgram(0);
}

PluginSimpleSynth::~PluginSimpleSynth() {
    delete env;
    delete osc;
}

// -----------------------------------------------------------------------
// Init

void PluginSimpleSynth::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;

    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.1f;
    parameter.hints = kParameterIsAutomable | kParameterIsLogarithmic;

    switch (index) {
        case paramVolume:
            parameter.name = "Volume";
            parameter.symbol = "volume";
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
            // nothing to do here...
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
            setParameterValue(paramVolume, 0.6f);
            break;
    }
}

// -----------------------------------------------------------------------
// Process

void PluginSimpleSynth::activate() {
    double fSampleRate = getSampleRate();
    osc->setFrequency(440.0f / fSampleRate);

    env->setAttackRate(0.1f * fSampleRate);
    env->setDecayRate(0.3f * fSampleRate);
    env->setReleaseRate(2.0f * fSampleRate);
    env->setSustainLevel(0.8f);

    for (int i=0; i < 128; i++) {
        noteState[i] = false;
    }
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
                            env->gate(false);
                        }
                    }
                    else if (velo > 0) {
                        freq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
                        osc->setFrequency(freq / fSampleRate);
                        noteState[note] = true;
                        env->gate(true);
                        break;
                    }
                    break;
                case 0x80:
                    note = data[1];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (noteState[note]) {
                        noteState[note] = false;
                        env->gate(false);
                    }
                    break;
            }
        }

        if (curEventIndex < midiEventCount && midiEvents[curEventIndex].frame < frames)
            count = midiEvents[curEventIndex].frame - pos;
        else
            count = frames - pos;

        for (uint32_t i=0; i<count; ++i) {
            float sample = osc->getOutput() * env->process() * vol;
            outL[pos + i] = sample;
            outR[pos + i] = sample;
            osc->updatePhase();
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
