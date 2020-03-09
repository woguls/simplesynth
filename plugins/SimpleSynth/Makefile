#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = simplesynth

# --------------------------------------------------------------
# Use the csound api

LDFLAGS += -lcsound64

# --------------------------------------------------------------
# Files to build

OBJS_DSP = \
	CsoundSession.cpp.o \
	PluginCsound.cpp.o

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
