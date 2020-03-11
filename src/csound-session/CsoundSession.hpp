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

private:
	CSOUND_PARAMS m_csParams;
	String m_orc;

};

END_NAMESPACE_DISTRHO
#endif