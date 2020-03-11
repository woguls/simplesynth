

/*
 * csoundlv2 audio effect based on DISTRHO Plugin Framework (DPF)
 *
 * SPDX-License-Identifier: MIT
 *
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

#ifndef PLUGIN_CSOUNDLV2_H
#define PLUGIN_CSOUNDLV2_H

#include "DistrhoPlugin.hpp"
#include "CsoundSession.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class Plugincsoundlv2 : public Plugin {
public:
    // Dummy list of params for future reference. paramFreq is not used by this plugin
    enum Parameters {
    paramFrequency=0,
        paramVolume,
        
        paramCount
    };

    Plugincsoundlv2();

    ~Plugincsoundlv2();
protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override {
        return "csound-simplesynth";
    }

    const char* getDescription() const override {
        return "a simple synth made with csound";
    }

    const char* getMaker() const noexcept override {
        return "David DeWert";
    }

    const char* getHomePage() const override {
        return "https://github.com/woguls/simplesynth/tree/csound";
    }

    const char* getLicense() const noexcept override {
        return "https://spdx.org/licenses/MIT";
    }

    uint32_t getVersion() const noexcept override {
        return d_version(0, 1,0);
    }

    // Go to:
    //
    // http://service.steinberg.de/databases/plugin.nsf/plugIn
    //
    // Get a proper plugin UID and fill it in here!
    int64_t getUniqueId() const noexcept override {
        return d_cconst('a', 'b', 'c', 'd');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameterList(Parameter** parameterList);
    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    void loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // Optional

    // Optional callback to inform the plugin about a sample rate change.
    void sampleRateChanged(double newSampleRate) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;

    void run(const float**, float** outputs, uint32_t frames,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override;


    // -------------------------------------------------------------------

private:

    float    fParams[paramCount];
    double   fSampleRate;
    int      processedFrames;
    int      result;
    int      ksmps;
    MYFLT    scale;
    bool noteState[128];

    // params[] contains a list of the parameters
    // the names of the parameters are needed for csound chanset/get
    Parameter* params[paramCount];
    bool paramShouldSend[paramCount];
    bool paramShouldReceive[paramCount];
    CsoundSession* cs;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Plugincsoundlv2)
};

struct Preset {
    const char* name;
    float params[Plugincsoundlv2::paramCount];
};

const Preset factoryPresets[] = {
    
    {
        "Default",
        {440.1f}
    }
        ,
    {
        "Default",
        {0.5f}
    }
        
};

const uint presetCount = sizeof(factoryPresets) / sizeof(Preset);

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // #ifndef PLUGIN_CSOUNDLV2_H
