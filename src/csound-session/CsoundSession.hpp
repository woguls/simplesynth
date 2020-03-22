#ifndef PLUGIN_CSOUND_SESSION_H
#define PLUGIN_CSOUND_SESSION_H

#include <csound/csound.hpp>

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class AudioBuffers
{
public:
    AudioBuffers(const MYFLT zdbfs, const uint32_t ksmps, MYFLT* spin, MYFLT* spout, CSOUND * const csound);
    void Copy(const uint32_t low, const uint32_t high, const float** const in, float** const out);

private:
    const MYFLT m_0dBFS;
    MYFLT * const m_spin;
    MYFLT * const m_spout;
    const uint32_t m_ksmps;
    uint32_t m_processedFrames;
    CSOUND * const m_csound;
};


class CsoundSession : public Csound
{
public:	
	CsoundSession(const char* orc, const double framerate, const uint32_t buffersize);

    void CopyBuffers(uint32_t low, uint32_t high, const float** in, float** out);
private:
	CSOUND_PARAMS m_csParams;
	String m_orc;
    int m_result;
    AudioBuffers* buffers;
};


END_NAMESPACE_DISTRHO
#endif