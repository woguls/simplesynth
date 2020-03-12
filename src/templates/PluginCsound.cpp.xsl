<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
<xsl:template match="/">

/*
 * csoundlv2 audio effect based on DISTRHO Plugin Framework (DPF)
 *
 * SPDX-License-Identifier: MIT
 * 
 * Copyright (C) 2018 Christopher Arndt &lt;info@chrisarndt.de&gt;
 * Copyright (C) 2020 David DeWert &lt;daviddewert@gmail.com&gt;
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the &quot;Software&quot;), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED &quot;AS IS&quot;, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include &lt;math.h&gt;

#include &quot;PluginCsound.out.hpp&quot;

START_NAMESPACE_DISTRHO

const char orc1[] = { {ORC_TEXT} , 0x00 };


// -----------------------------------------------------------------------

Plugincsoundlv2::Plugincsoundlv2()
    : Plugin(paramCount, presetCount, 0)  // paramCount param(s), presetCount program(s), 0 states
{
    for (int i=0; i &lt; 128; i++) {
        noteState[i] = false;
    }

    initParameterList(params);

    if (presetCount &gt; 0) {
        loadProgram(0);
    }

    cs = new CsoundSession{orc1};
}

Plugincsoundlv2::~Plugincsoundlv2() {
    delete cs;
    for (int i = 0; i &lt; paramCount; i++) {
        delete params[i];
    }
}
// -----------------------------------------------------------------------
// Init

void Plugincsoundlv2::initParameter(uint32_t index, Parameter&amp; parameter) {
    if (index &gt;= paramCount)
        return;

    /**
     * see Plugincsoundlv2::Parameters for the list of parameters
    */
    switch (index) {
<xsl:for-each select="/plugin/parameter">
        case <xsl:value-of select="symbol"/>:
            parameter.name = &quot;<xsl:value-of select="name"/>&quot;;
            parameter.symbol = &quot;<xsl:value-of select="symbol"/>&quot;;
            parameter.ranges.min = <xsl:value-of select="min"/>;
            parameter.ranges.max = <xsl:value-of select="max"/>;
            parameter.ranges.def = <xsl:value-of select="def"/>;
            parameter.hints = 
    <xsl:for-each select="hint">
        <xsl:value-of select="."/> <xsl:if test="position() != last()"> | </xsl:if>
    </xsl:for-each>;
            break;

</xsl:for-each>
    }
}

/**
  We need a convenient place to get the parameter symbols for use with csound&apos;s chanset
*/
void Plugincsoundlv2::initParameterList(Parameter** parameterList) {
    int paramindex;
    for (paramindex = 0; paramindex &lt; paramCount; paramindex++ ) {
        params[paramindex] = new Parameter;
        initParameter(paramindex, *params[paramindex]);
        paramShouldSend[paramindex] = true;
        paramShouldReceive[paramindex] = false;
    }
}
/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void Plugincsoundlv2::initProgramName(uint32_t index, String&amp; programName) {
    if (index &lt; presetCount) {
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
    paramShouldSend[index] = true;
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void Plugincsoundlv2::loadProgram(uint32_t index) {
    if (index &lt; presetCount) {
        for (int i=0; i &lt; paramCount; i++) {
            setParameterValue(i, factoryPresets[index].params[i]);
            paramShouldSend[i] = true;
        }
    }
}

// -----------------------------------------------------------------------
// Process

void Plugincsoundlv2::activate() {
    // plugin is activated
    result = cs-&gt;Init(getSampleRate(), getBufferSize());
    scale = cs-&gt;Get0dBFS();
    ksmps = cs-&gt;GetKsmps();
    processedFrames = ksmps;
    for (int i=0; i &lt; paramCount; i++) {
        paramShouldSend[i] = true;
    }
}



void Plugincsoundlv2::run(const float** inputs, float** outputs,
                          uint32_t frames,
                          const MidiEvent* midiEvents, uint32_t midiEventCount) {

    uint8_t note, velo;

    // this is probably unnecessary but should ensure the compiler will unroll the loop. Which is itself probably not necessary...
    constexpr int pcount = paramCount;
    
    for(int p=0; p&lt;pcount; p++) {
        if (paramShouldSend[p]) {
            cs-&gt;SetChannel( params[p]-&gt;symbol.buffer(), fParams[p]);
            paramShouldSend[p] = false;
            printf(&quot;sending %s = %f\n&quot;, params[p]-&gt;symbol.buffer(), fParams[p]);
            paramShouldSend[p] = false;
        }
    }

    for (uint32_t count, pos=0, curEventIndex=0; pos&lt;frames;) {
        for (;curEventIndex &lt; midiEventCount &amp;&amp; pos &gt;= midiEvents[curEventIndex].frame; ++curEventIndex) {
            if (midiEvents[curEventIndex].size &gt; MidiEvent::kDataSize)
                continue;

            const uint8_t* data = midiEvents[curEventIndex].data;
            const uint8_t status = data[0] &amp; 0xF0;

            switch (status) {
                case 0x90: // note on
                    note = data[1];
                    velo = data[2];
                    DISTRHO_SAFE_ASSERT_BREAK(note &lt; 128);

                    if (noteState[note]) {
                        if (velo == 0) {

                            DISTRHO_SAFE_ASSERT_BREAK(note &lt; 128);

                            if (noteState[note]) {
                                noteState[note] = false;
                                float inum = -1 *(1 + (float)note/1000.0f);
                                const MYFLT send[5] = {inum, 0, -1, 0, float(note)};
                                cs-&gt;ScoreEvent(&apos;i&apos;, send, 5);
                            }
                        }
                    }
                    else if (velo &gt; 0) {
                        noteState[note] = true;
                        float inum = 1 + (float)note/1000.0f;
                        const MYFLT send[5] = {inum, 0, -1, velo, float(note) } ;
                        cs-&gt;ScoreEvent(&apos;i&apos;, send, 5);
                        break;
                    }
                    break;
                case 0x80: // note off
                    note = data[1];
                    DISTRHO_SAFE_ASSERT_BREAK(note &lt; 128);

                    if (noteState[note]) {
                        noteState[note] = false;
                        // each note should have a unique number of the form x.xxx
                        float inum = -1 * (1 + (float)note/1000.0f);
                        const MYFLT send[5] = {inum, 0, -1, 0, float(note)};
                        cs-&gt;ScoreEvent(&apos;i&apos;, send, 5);
                    }
                    break;
            }
        }

        if (curEventIndex &lt; midiEventCount &amp;&amp; midiEvents[curEventIndex].frame &lt; frames)
            count = midiEvents[curEventIndex].frame - pos;
        else
            count = frames - pos;

        for (uint32_t i=0; i&lt;count; ++i, processedFrames++) {
            if ( processedFrames == ksmps &amp;&amp; result == 0) {
                result = cs-&gt;PerformKsmps();
                processedFrames = 0;
            }
            for (int j = 0; j &lt; DISTRHO_PLUGIN_NUM_OUTPUTS; j++) {
                if ( result == 0) {
                    MYFLT out = cs-&gt;GetSpoutSample(processedFrames,j);
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

</xsl:template>
</xsl:stylesheet>