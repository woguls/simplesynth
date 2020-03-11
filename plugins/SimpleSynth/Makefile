#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = $(shell xmlstarlet sel -t -v "/plugin/name" plugin.xml)

# --------------------------------------------------------------
# Use the csound api

LDFLAGS += -lcsound64

$(shell xsltproc ../../src/templates/DistrhoPluginInfo.h.xsl ./plugin.xml >  ./DistrhoPluginInfo.h)	

# --------------------------------------------------------------
# Files to build
.PHONY: PluginCsound.out.cpp
PluginCsound.out.cpp:
	$(shell xsltproc ../../src/templates/PluginCsound.cpp.xsl ./plugin.xml >  ./PluginCsound-temp.out.cpp)
	$(shell xsltproc ../../src/templates/PluginCsound.hpp.xsl ./plugin.xml >  ./PluginCsound.out.hpp)
	$(call generate_plugin_csound, ./orc.csd, PluginCsound-temp.out.cpp, ./PluginCsound.out.cpp) 

OBJS_DSP = \
	../../src/csound-session/CsoundSession.cpp.o \
	PluginCsound.out.cpp.o

# --------------------------------------------------------------
# Do some magic

include ../Makefile.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(LINUX),true)
#TARGETS += dssi ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

all: $(TARGETS)
	
	

# --------------------------------------------------------------
