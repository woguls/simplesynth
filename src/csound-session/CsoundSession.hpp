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

    void Run(uint32_t pos, const float** in, float** out) {

    	if (m_processedFrames == GetKsmps() ) {
    		PerformKsmps();
    		m_processedFrames = 0;
    	}

          // outp[j][i] = (LADSPA_Data) (spout[j+pos]/scale);
    	for (uint8_t j = 0; j < DISTRHO_PLUGIN_NUM_OUTPUTS; j++) {
            const MYFLT sample = GetSpoutSample(m_processedFrames, j);
            out[j][pos] = float(sample / Get0dBFS());
    	}
    	for (uint8_t j = 0; j < DISTRHO_PLUGIN_NUM_INPUTS; j++) {
            uint8_t offset = m_processedFrames * DISTRHO_PLUGIN_NUM_INPUTS;
            m_spin[j + offset] = in[j][pos] * Get0dBFS();
            // AddSpinSample(m_processedFrames, j, MYFLT( in[j][pos])*Get0dBFS());
    	}
    	m_processedFrames++;
    }

private:
	CSOUND_PARAMS m_csParams;
	String m_orc;
	int m_processedFrames;
    MYFLT *m_spin, *m_spout;
};

END_NAMESPACE_DISTRHO
#endif