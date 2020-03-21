#include "CsoundSession.hpp"

START_NAMESPACE_DISTRHO



void noMessageCallback(CSOUND*, int, const char *format, va_list valist)
{
  // Do nothing so that Csound will not print any message,
  // leaving a clean console for our app
	vprintf(format, valist);
  return;
}


// -----------------------------------------------------------------------
// CopyBuffers
// low, high: sample numbers relative to the Distrho buffers.
void CsoundSession::CopyBuffers(uint32_t low, uint32_t high, const float** in, float** out) {

}

void CsoundSession::Run(uint32_t pos, const float** in, float** out) {

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

int CsoundSession::Init(int framerate, int buffersize) {

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
		m_processedFrames = GetKsmps();
	}
	else {
	  return 1;
	}
	
	m_spout = GetSpout();
  	m_spin  = GetSpin();
	return 0;
}



END_NAMESPACE_DISTRHO