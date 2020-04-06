#include "CsoundSession.hpp"
#include <string.h>

START_NAMESPACE_DISTRHO

CsoundSession::MidiUserData CsoundSession::g_midiUserData;

int CsoundSession::MidiInDeviceOpen(CSOUND *csound, void **userData, const char *dev) {
    g_midiUserData.midiBuffer = csoundCreateCircularBuffer(csound, 4096, sizeof(unsigned char));
    g_midiUserData.virtualMidiBuffer = csoundCreateCircularBuffer(csound,4096, sizeof(unsigned char));

    *userData = (void *) &g_midiUserData;
    printf("MidiInDeviceOpen called with %s\n", dev);
    return CSOUND_SUCCESS;
}

int CsoundSession::MidiDataRead(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    MidiUserData* const ud = static_cast<MidiUserData*>(userData);

    for (uint32_t eventc = ud->start; eventc < ud->end; eventc++) {
        // copy one event
        ud->start++;

        const MidiEvent evt = ud->events[eventc];

        // if we've run out of space in mbuf
        if (evt.size > nbytes) continue;

        memcpy(mbuf , evt.data, evt.size * sizeof(unsigned char) );
        return evt.size;
    }
        
    return 0;
}

int CsoundSession::MidiInDeviceClose(CSOUND *csound, void *userData) {
    return 0;
}

const char* CsoundSession::MidiErrorString( int e) {
    return "midi error\n";
}

void noMessageCallback(CSOUND*, int, const char *format, va_list valist)
{
  // Do nothing so that Csound will not print any message,
  // leaving a clean console for our app
	vprintf(format, valist);
  return;
}

CsoundSession::~CsoundSession() {
    delete buffers;
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
    m_csParams.number_of_threads = CSOUND_NUM_THREADS;

    // Note that setParams is called before first compilation
    SetParams(&m_csParams);
    SetOption("-n");
    SetOption("-d");
    SetOption("-+rtmidi=hostbased");
    SetOption("-Ma");
    SetExternalMidiInOpenCallback(MidiInDeviceOpen);
    SetExternalMidiReadCallback(MidiDataRead);
    SetExternalMidiInCloseCallback(MidiInDeviceClose);
    SetExternalMidiErrorStringCallback(MidiErrorString);
    SetHostImplementedMIDIIO(1);
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