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
        noteStack[i] = -1;
    }

    noteStackPos = -1;
}


/**
   Handy class to help keep audio buffer in sync with incoming MIDI events.
   To use it, create a local variable (on the stack) and call nextEvent() until it returns false.
   @code
    for (AudioMidiSyncHelper amsh(outputs, frames, midiEvents, midiEventCount); amsh.nextEvent();)
    {
        float* const outL = amsh.outputs[0];
        float* const outR = amsh.outputs[1];

        for (uint32_t i=0; i<amsh.midiEventCount; ++i)
        {
            const MidiEvent& ev(amsh.midiEvents[i]);
            // ... do something with the midi event
        }

        renderSynth(outL, outR, amsh.frames);
    }
   @endcode

   Some important notes when using this class:
    1. MidiEvent::frame retains its original value, but it is useless, do not use it.
    2. The class variables names are the same as the parameter names of the run function.
       Keep that in mind and try to avoid typos. :)
 */
class AudioMidiSyncHelper {
public:
    /** Parameters from the run function, adjusted for event sync */
    float** outputs;
    uint32_t frames;
    const MidiEvent* midiEvents;
    uint32_t midiEventCount;

    /**
       Constructor, using values from the run function.
    */
    AudioMidiSyncHelper(float** const o, uint32_t f, const MidiEvent* m, uint32_t mc)
        : outputs(o),
          frames(0),
          midiEvents(m),
          midiEventCount(0),
          remainingFrames(f),
          remainingMidiEventCount(mc) {}

    /**
       Process a batch of events untill no more are available.
       You must not read any more values from this class after this function returns false.
    */
    bool nextEvent()
    {
        if (remainingMidiEventCount == 0)
        {
            if (remainingFrames == 0)
                return false;

            for (uint32_t i=0; i<DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
                outputs[i] += frames;

            if (midiEventCount != 0)
                midiEvents += midiEventCount;

            frames = remainingFrames;
            midiEvents = nullptr;
            midiEventCount = 0;
            remainingFrames = 0;
            return true;
        }

        const uint32_t eventFrame = midiEvents[0].frame;
        DISTRHO_SAFE_ASSERT_RETURN(eventFrame < remainingFrames, false);

        midiEventCount = 1;

        for (uint32_t i=1; i<remainingMidiEventCount; ++i)
        {
            if (midiEvents[i].frame != eventFrame)
            {
                midiEventCount = i;
                break;
            }
        }

        frames = remainingFrames - eventFrame;
        remainingFrames -= frames;
        remainingMidiEventCount -= midiEventCount;
        return true;
    }

private:
    /** @internal */
    uint32_t remainingFrames;
    uint32_t remainingMidiEventCount;
};


void PluginSimpleSynth::run(const float**, float** outputs, uint32_t frames,
                            const MidiEvent *midiEvents, uint32_t midiEventCount) {
    uint8_t note, velo;
    float freq, vol = fParams[paramVolume];

    // Loop over MIDI events in this block in batches grouped by event frame number,
    // i.e. each batch has a number of events occuring at the same frame number.
    // First we need to make an instance of sync helper in the for-loop initializer.
    // The for-loop break condition is amsh.nextEvent(), when it returns null,
    // there are no more events and the loop ends.
    //
    // One could also use a while-loop like this:
    //
    // AudioMidiSyncHelper amsh(outputs, frames, midiEvents, midiEventCount);
    //
    // while (amsh.nextEvent()) {
    //    ...
    // }
    for (AudioMidiSyncHelper amsh(outputs, frames, midiEvents, midiEventCount); amsh.nextEvent();) {
        // Loop over events in this batch
        for (uint32_t i=0; i<amsh.midiEventCount; ++i) {
            if (amsh.midiEvents[i].size > MidiEvent::kDataSize)
                // Skip MIDI events with more than 3 bytes.
                continue;

            const uint8_t* data = amsh.midiEvents[i].data;
            // Determine MIDI event status by masking out MIDI channel in lower four bits.
            const uint8_t status = data[0] & 0xF0;

            switch (status) {
                case 0x90:
                    // Note On
                    note = data[1];
                    velo = data[2];
                    // Make sure note number is within range 0 .. 127
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (velo > 0) {
                        noteState[note] = true;

                        if (noteStackPos >= 0 && note == noteStack[noteStackPos])
                            break;

                        if (noteStackPos == -1) {
                            env->gate(true);
                        }

                        noteStack[++noteStackPos] = note;
                        freq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
                        osc->setFrequency(freq / fSampleRate);
                        break;
                    }
                    // Fall-through for Note On with velocity 0 => Note Off
                case 0x80:
                    note = data[1];
                    noteState[note] = false;

                    // No note currently playing
                    if (noteStackPos == -1)
                        break;

                    // Note off for currently playing note
                    if (noteStack[noteStackPos] == note) {
                        // While note stack is not empty and previous held note already released
                        while (noteStackPos >= 0 && !noteState[note]) {
                            // Get next previously held note
                            note = noteStack[--noteStackPos];
                        }

                        if (noteStackPos >= 0) {
                            // A note is still held
                            freq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
                            osc->setFrequency(freq / fSampleRate);
                        }
                        else {
                            // No notes held anymore
                            env->gate(false);
                        }
                    }

                    break;
            }
        }

        // Get the left and right audio outputs from the AudioMidiSyncHelper.
        // This ensures that the pointer is positioned at the right frame within
        // the block for the current batch.
        float* const outL = amsh.outputs[0];
        float* const outR = amsh.outputs[1];

        // Write samples for the frames of the current batch
        for (uint32_t i=0; i<amsh.frames; ++i) {
            float sample = osc->getOutput() * env->process() * vol;
            outL[i] = sample;
            outR[i] = sample;
            osc->updatePhase();
        }
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new PluginSimpleSynth();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
