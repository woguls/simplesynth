#include "CsoundSession.hpp"
#include <string.h>

START_NAMESPACE_DISTRHO

CsoundSession::MidiUserData CsoundSession::g_midiUserData;

int CsoundSession::MidiInDeviceOpen(CSOUND *csound, void **userData, const char *dev) {

    *userData = (void *) &g_midiUserData;
    //printf("MidiInDeviceOpen called with %s\n", dev);
    return CSOUND_SUCCESS;
}


// copy one midi event to mbuf from dhistro buffer, return the length of the event, or return 0 if there are no more events in the distrho buffer
// apparently durring performKsmps() csound will call this until it returns 0
int CsoundSession::MidiDataRead(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    MidiUserData* const ud = static_cast<MidiUserData*>(userData);

    if (ud->start < ud->end) {
    }
    for (uint32_t eventc = ud->start; eventc < ud->end; eventc++) {
        // copy one event
        ud->start++;

        const MidiEvent evt = ud->events[eventc];

        // if we've run out of space in mbuf
        if (evt.size > nbytes) continue;
        //printf("midi: size %i, status %x, channel %x, data1 %x, data2 %x\n", evt.size, evt.data[0] & 0xF0, evt.data[0] & 0x0F, evt.data[1], evt.data[2]);
        memcpy(mbuf , evt.data, evt.size * sizeof(unsigned char) );
        return evt.size;
    }
    return CSOUND_SUCCESS;
}

int CsoundSession::MidiInDeviceClose(CSOUND *csound, void *userData) {
    return CSOUND_SUCCESS;
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
    SetOption("-M0");
    SetExternalMidiInOpenCallback(MidiInDeviceOpen);
    SetExternalMidiReadCallback(MidiDataRead);
    SetExternalMidiInCloseCallback(MidiInDeviceClose);
    SetExternalMidiErrorStringCallback(MidiErrorString);
    SetHostImplementedMIDIIO(1);
    m_result = CompileOrc(m_orc);
    if ( m_result == CSOUND_SUCCESS) {
        Start();
        buffers = new AudioBuffers(Get0dBFS(), GetKsmps(), GetSpin(), GetSpout(), GetCsound());
    }
};
// -----------------------------------------------------------------------
// CopyBuffers
// low, high: sample numbers relative to the Distrho buffers.
void CsoundSession::CopyBuffers(const uint32_t low, const uint32_t high, const float** in, float** out) {
    if (m_result == CSOUND_SUCCESS) m_result = buffers->Copy(low, high, in, out);
}

AudioBuffers::AudioBuffers(const MYFLT zdbfs, const uint32_t ksmps, MYFLT* spin, MYFLT* spout, CSOUND * const csound) :
        m_0dBFS{zdbfs},
        m_ksmps{ksmps},
        m_spin{spin},
        m_spout{spout},
        m_processedFrames{ksmps},
        m_csound{csound}
        {}

int AudioBuffers::Copy(const uint32_t low, const uint32_t high, const float** const in, float** const out) {
    int result = CSOUND_SUCCESS;
	for (uint32_t frame = low; frame < high; frame++, m_processedFrames++) {
		if (m_processedFrames == m_ksmps ) {
			result = csoundPerformKsmps(m_csound);
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
    return result;
}

END_NAMESPACE_DISTRHO