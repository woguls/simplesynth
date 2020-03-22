#include "CsoundSession.hpp"

START_NAMESPACE_DISTRHO



void noMessageCallback(CSOUND*, int, const char *format, va_list valist)
{
  // Do nothing so that Csound will not print any message,
  // leaving a clean console for our app
	vprintf(format, valist);
  return;
}

CsoundSession::CsoundSession(const char* orc, double framerate, uint32_t buffersize) : Csound() {
	m_orc = orc;
    SetMessageCallback(noMessageCallback);
    GetParams(&m_csParams);

    m_csParams.sample_rate_override = framerate;
    m_csParams.control_rate_override = framerate/buffersize;
    m_csParams.e0dbfs_override = 1.0;
    m_csParams.nchnls_override = DISTRHO_PLUGIN_NUM_OUTPUTS;
    m_csParams.nchnls_i_override = DISTRHO_PLUGIN_NUM_INPUTS;
    m_csParams.debug_mode = 0;
    m_csParams.realtime_mode = 1;


    // Note that setParams is called before first compilation
    SetParams(&m_csParams);
    SetOption("-n");
    SetOption("-d");
    if (CompileOrc(m_orc) == 0) {
        Start();
    }
    else {
      m_result = 1;
      return;
    }
    
    m_result =  0;

    buffers = new AudioBuffers(Get0dBFS(), GetKsmps(), GetSpin(), GetSpout(), GetCsound());
};
// -----------------------------------------------------------------------
// CopyBuffers
// low, high: sample numbers relative to the Distrho buffers.
void CsoundSession::CopyBuffers(const uint32_t low, const uint32_t high, const float** in, float** out) {

	buffers->Copy(low, high, in, out);
}

AudioBuffers::AudioBuffers(const MYFLT zdbfs, const uint32_t ksmps, MYFLT* spin, MYFLT* spout, CSOUND * const csound) :
        m_0dBFS{zdbfs},
        m_ksmps{ksmps},
        m_spin{spin},
        m_spout{spout},
        m_processedFrames{ksmps},
        m_csound{csound}
        {}

void AudioBuffers::Copy(const uint32_t low, const uint32_t high, const float** const in, float** const out) {
	for (uint32_t frame = low; frame < high; frame++, m_processedFrames++) {
		if (m_processedFrames == m_ksmps ) {
			csoundPerformKsmps(m_csound);
			m_processedFrames = 0;
		}

		for (uint32_t j = 0; j < DISTRHO_PLUGIN_NUM_OUTPUTS; j++) {
			const uint32_t offset = m_processedFrames * DISTRHO_PLUGIN_NUM_OUTPUTS;
	        out[j][frame] = float(m_spout[j + offset] / m_0dBFS);
		}

		for (uint32_t j = 0; j < DISTRHO_PLUGIN_NUM_INPUTS; j++) {
	        const uint32_t offset = m_processedFrames * DISTRHO_PLUGIN_NUM_INPUTS;
	        m_spin[j + offset] = in[j][frame] * m_0dBFS;
		}
	}

}

END_NAMESPACE_DISTRHO