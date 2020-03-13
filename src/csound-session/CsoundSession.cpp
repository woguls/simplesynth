#include "CsoundSession.hpp"

START_NAMESPACE_DISTRHO



void noMessageCallback(CSOUND* cs, int attr, const char *format, va_list valist)
{
  // Do nothing so that Csound will not print any message,
  // leaving a clean console for our app
	vprintf(format, valist);
  return;
}

int CsoundSession::Init(int framerate, int buffersize) {

	SetMessageCallback(noMessageCallback);
	GetParams(&m_csParams);

	m_csParams.sample_rate_override = framerate;
	m_csParams.control_rate_override = framerate/buffersize;
	m_csParams.e0dbfs_override = 1.0;
	m_csParams.nchnls_override = DISTRHO_PLUGIN_NUM_OUTPUTS;
	m_csParams.debug_mode = 0;
	m_csParams.realtime_mode = 1;
	m_csParams.sample_accurate = 1;

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
	
	return 0;
}



END_NAMESPACE_DISTRHO