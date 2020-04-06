<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
<xsl:template match="/">

/*
 * csoundlv2 audio effect based on DISTRHO Plugin Framework (DPF)
 *
 * SPDX-License-Identifier: MIT
 *
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

#ifndef PLUGIN_CSOUNDLV2_H
#define PLUGIN_CSOUNDLV2_H

#include &quot;DistrhoPlugin.hpp&quot;
#include &quot;CsoundSession.hpp&quot;

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class Plugincsoundlv2 : public Plugin {
public:
    // Dummy list of params for future reference. paramFreq is not used by this plugin
    enum Parameters {
    <xsl:for-each select="/plugin/parameter">
        <xsl:choose>

        <xsl:when test="position() = 1">
            <xsl:value-of select="symbol"/>=0,
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="symbol"/>,
        </xsl:otherwise>

        </xsl:choose>
    </xsl:for-each>
        paramCount
    };

    Plugincsoundlv2();

    ~Plugincsoundlv2();
protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override {
        return &quot;<xsl:value-of select="/plugin/label"/>&quot;;
    }

    const char* getDescription() const override {
        return &quot;<xsl:value-of select="/plugin/description"/>&quot;;
    }

    const char* getMaker() const noexcept override {
        return &quot;<xsl:value-of select="/plugin/maker"/>&quot;;
    }

    const char* getHomePage() const override {
        return &quot;<xsl:value-of select="/plugin/homepage"/>&quot;;
    }

    const char* getLicense() const noexcept override {
        return &quot;<xsl:value-of select="/plugin/license"/>&quot;;
    }

    uint32_t getVersion() const noexcept override {
        return d_version(<xsl:value-of select="/plugin/version/major"/>, <xsl:value-of select="/plugin/version/minor"/>,<xsl:value-of select="/plugin/version/patch"/>);
    }

    // Go to:
    //
    // http://service.steinberg.de/databases/plugin.nsf/plugIn
    //
    // Get a proper plugin UID and fill it in here!
    int64_t getUniqueId() const noexcept override {
        return d_cconst(&apos;z&apos;, &apos;b&apos;, &apos;c&apos;, &apos;d&apos;);
    }

    // -------------------------------------------------------------------
    // Init

    void initParameterList();
    void initParameter(uint32_t index, Parameter&amp; parameter) override;
    void initProgramName(uint32_t index, String&amp; programName) override;

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

    void run0(
        const float**,
        float** outputs,
        uint32_t frames
        <xsl:if test="(/plugin/distrho/midiinput &gt; 0) or (/plugin/distrho/midioutput &gt; 0)">
            <xsl:text>,const MidiEvent* midiEvents,
            uint32_t midiEventCount</xsl:text>
        </xsl:if>
    );

    void run(
        const float**,
        float** outputs,
        uint32_t frames
        <xsl:if test="(/plugin/distrho/midiinput &gt; 0) or (/plugin/distrho/midioutput &gt; 0)">
            <xsl:text>,const MidiEvent* midiEvents,
            uint32_t midiEventCount</xsl:text>
        </xsl:if>
    ) override;

    // -------------------------------------------------------------------

private:

    float    fParams[paramCount];
    double   fSampleRate;
    int      result;
    // int      ksmps;
    // MYFLT    scale;
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
        &quot;Default&quot;,
        {
        <xsl:for-each select="/plugin/parameter">

        <xsl:value-of select="def"/>
        <xsl:if test="position() != last()">
             <xsl:text>,</xsl:text>
        </xsl:if>
    </xsl:for-each>
        }
    }
};

const uint presetCount = sizeof(factoryPresets) / sizeof(Preset);

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // #ifndef PLUGIN_CSOUNDLV2_H
</xsl:template>
</xsl:stylesheet>