#ifndef PLUGIN_CSOUND_SESSION_H
#define PLUGIN_CSOUND_SESSION_H

#include <csound/csound.hpp>

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

class AudioBuffers
{
public:
    AudioBuffers(const MYFLT zdbfs, const uint32_t ksmps, MYFLT* spin, MYFLT* spout, CSOUND * const csound);
    int Copy(const uint32_t low, const uint32_t high, const float** const in, float** const out);
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
    ~CsoundSession();
    
    void CopyBuffers(uint32_t low, uint32_t high, const float** in, float** out);

    void ResetMidiIndex() { g_midiUserData.start = 0; g_midiUserData.end = 0; }
    void SetMidiIndexStart(uint32_t start) { g_midiUserData.start = start; }
    void SetMidiIndexEnd(uint32_t end) { g_midiUserData.end = end; }
    void SetMidiEventsPtr( const MidiEvent* p) { g_midiUserData.events = p; }

    struct MidiUserData {
        uint32_t start = 0;
        uint32_t end = 0;
        uint32_t eventCount = 0;
        const MidiEvent* events;
    };

    void SetMidiUserData(const MidiEvent* midiEvents, const uint32_t midiEventCount) {
        g_midiUserData.events = midiEvents;
        g_midiUserData.eventCount = midiEventCount;
        g_midiUserData.start = 0;
        g_midiUserData.end = 0;
    }


private:
    static int MidiInDeviceOpen(CSOUND *csound, void **userData, const char *dev);
    static int MidiDataRead(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
    static int MidiInDeviceClose(CSOUND *csound, void *userData);
    static const char* MidiErrorString(int e);
    static MidiUserData g_midiUserData;

	CSOUND_PARAMS m_csParams;
	String m_orc;
    int m_result;
    AudioBuffers* buffers;
};


END_NAMESPACE_DISTRHO
#endif