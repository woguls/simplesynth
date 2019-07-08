#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = simplesynth

# --------------------------------------------------------------
# Files to build

OBJS_DSP = \
	ADSR.cpp.o \
	LFO.cpp.o \
	LPF.cpp.o \
	WaveUtils.cpp.o \
	PluginSimpleSynth.cpp.o

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
