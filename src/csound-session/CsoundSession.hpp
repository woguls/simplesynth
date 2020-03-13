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

    template<uint8_t NCHAN>
    void Run(uint32_t pos, float** out) {
    	if (m_processedFrames == GetKsmps() ) {
    		PerformKsmps();
    		m_processedFrames = 0;
    	}

    	for (uint8_t j = 0; j < NCHAN; j++) {
    		const MYFLT sample = GetSpoutSample(m_processedFrames, j);
    		out[j][pos] = float(sample / Get0dBFS());
    	}
    	m_processedFrames++;
    }

private:
	CSOUND_PARAMS m_csParams;
	String m_orc;
	int m_processedFrames;

};

END_NAMESPACE_DISTRHO
#endif