#ifndef PLUGIN_CSOUND_SESSION_H
#define PLUGIN_CSOUND_SESSION_H

#include <csound/csound.hpp>

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO


class CsoundSession : public Csound
{
public:	
	CsoundSession(const char* orc) : Csound() {
    	m_orc = orc;
    };

    int Init(int framerate, int buffersize);

    void Run(uint32_t pos, const float** in, float** out);

    void CopyBuffers(uint32_t low, uint32_t high, const float** in, float** out);
private:
	CSOUND_PARAMS m_csParams;
	String m_orc;
	int m_processedFrames;
    MYFLT *m_spin, *m_spout;
};

END_NAMESPACE_DISTRHO
#endif