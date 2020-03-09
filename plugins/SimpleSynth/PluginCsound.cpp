/*
 * csoundlv2 audio effect based on DISTRHO Plugin Framework (DPF)
 *
 * SPDX-License-Identifier: MIT
 * 
 * Copyright (C) 2018 Christopher Arndt <info@chrisarndt.de>
 * Copyright (C) 2020 David DeWert <daviddewert@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <math.h>

#include "PluginCsound.hpp"

START_NAMESPACE_DISTRHO

const char* orc1 =
"instr 1              \n"
"iamp = p4/127 \n"
"ipch = cpsmidinn(p5)    \n"
"printk2 iamp \n"
"printk2 ipch \n"
"a1   poscil iamp, ipch \n"
"     outs    a1,a1         \n"
"endin";


// -----------------------------------------------------------------------

Plugincsoundlv2::Plugincsoundlv2()
    : Plugin(paramCount, presetCount, 0)  // paramCount param(s), presetCount program(s), 0 states
{
    for (int i=0; i < 128; i++) {
        noteState[i] = false;
    }

    initParameterList(params);

    if (presetCount > 0) {
        loadProgram(0);
    }

    cs = new CsoundSession{orc1};
}

Plugincsoundlv2::~Plugincsoundlv2() {
    delete cs;
    for (int i = 0; i < paramCount; i++) {
        delete params[i];
    }
}
// -----------------------------------------------------------------------
// Init

void Plugincsoundlv2::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;


    switch (index) {
        case paramFreq:
            parameter.name = "Frequency";
            parameter.symbol = "frequency";
            parameter.ranges.min = 10.0f;
            parameter.ranges.max = 10000.0f;
            parameter.ranges.def = 440.1f;
            parameter.hints = kParameterIsAutomable | kParameterIsLogarithmic;
            break;
    }
}

/**
  We need a convenient place to get the parameter symbols for use with csound's chanset
*/
void Plugincsoundlv2::initParameterList(Parameter** parameterList) {
    int paramindex;
    for (paramindex = 0; paramindex < paramCount; paramindex++ ) {
        params[paramindex] = new Parameter;
        initParameter(paramindex, *params[paramindex]);
    }
}
/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void Plugincsoundlv2::initProgramName(uint32_t index, String& programName) {
    if (index < presetCount) {
        programName = factoryPresets[index].name;
    }
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void Plugincsoundlv2::sampleRateChanged(double newSampleRate) {
    fSampleRate = newSampleRate;
}

/**
  Get the current value of a parameter.
*/
float Plugincsoundlv2::getParameterValue(uint32_t index) const {
    return fParams[index];
}

/**
  Change a parameter value.
*/
void Plugincsoundlv2::setParameterValue(uint32_t index, float value) {
    fParams[index] = value;

    switch (index) {
        case paramFreq:
            // do something when volume_r param is set
            break;
    }
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void Plugincsoundlv2::loadProgram(uint32_t index) {
    if (index < presetCount) {
        for (int i=0; i < paramCount; i++) {
            setParameterValue(i, factoryPresets[index].params[i]);
        }
    }
}

// -----------------------------------------------------------------------
// Process

void Plugincsoundlv2::activate() {
    // plugin is activated
    result = cs->Init(getSampleRate(), getBufferSize());
    scale = cs->Get0dBFS();
    processedFrames = cs->GetKsmps();
}



void Plugincsoundlv2::run(const float** inputs, float** outputs,
                          uint32_t frames,
                          const MidiEvent* midiEvents, uint32_t midiEventCount) {

    ksmps = cs->GetKsmps();


    uint8_t note, velo;
    // float freq, vol = fParams[paramVolume];

    for (uint32_t count, pos=0, curEventIndex=0; pos<frames;) {
        for (;curEventIndex < midiEventCount && pos >= midiEvents[curEventIndex].frame; ++curEventIndex) {
            if (midiEvents[curEventIndex].size > MidiEvent::kDataSize)
                continue;

            const uint8_t* data = midiEvents[curEventIndex].data;
            const uint8_t status = data[0] & 0xF0;

   /** From the csound api refererence:
     * Sends a score event of type 'opcod' (e.g. 'i' for a note event), with
     * 'pcnt' p-fields in array 'p' (p[0] is p1). If absp2mode is non-zero,
     * the start time of the event is measured from the beginning of
     * performance, instead of the default of relative to the current time.
    
    void ScoreEvent(int absp2mode, char opcod, int pcnt, const MYFLT *p);
*/
            switch (status) {
                case 0x90: // note on
                    note = data[1];
                    velo = data[2];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (noteState[note]) {
                        if (velo == 0) {

                            DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                            if (noteState[note]) {
                                noteState[note] = false;
                                float inum = -1 *(1 + (float)note/1000.0f);
                                const MYFLT send[5] = {inum, 0, -1, 0, float(note)};
                                cs->ScoreEvent('i', send, 5);
                            }
                        }
                    }
                    else if (velo > 0) {

                        noteState[note] = true;

                        float inum = 1 + (float)note/1000.0f;
                        const MYFLT send[5] = {inum, 0, -1, velo, float(note) } ;
                        cs->ScoreEvent('i', send, 5);
                        break;
                    }
                    break;
                case 0x80: // note off
                    note = data[1];
                    DISTRHO_SAFE_ASSERT_BREAK(note < 128);

                    if (noteState[note]) {
                        noteState[note] = false;
                        float inum = -1 * (1 + (float)note/1000.0f);
                        const MYFLT send[5] = {inum, 0, -1, 0, float(note)};
                        cs->ScoreEvent('i', send, 5);
                    }
                    break;
            }
        }

        if (curEventIndex < midiEventCount && midiEvents[curEventIndex].frame < frames)
            count = midiEvents[curEventIndex].frame - pos;
        else
            count = frames - pos;

        for (uint32_t i=0; i<count; ++i, processedFrames++) {
            if ( processedFrames == ksmps && result == 0) {
                result = cs->PerformKsmps();
                processedFrames = 0;
            }
            for (int j = 0; j < DISTRHO_PLUGIN_NUM_OUTPUTS; j++) {
                if ( result == 0) {
                    MYFLT out = cs->GetSpoutSample(processedFrames,j);
                    outputs[j][i] = float( out / scale);
                }
            }
        }
        pos += count;
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new Plugincsoundlv2();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
