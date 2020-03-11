# Simple Synth

A simple way to make lv2 plugins with csound

This project uses this [distrho plugin framework](https://github.com/DISTRHO/DPF/)

Build instructions and a better explanation of what the code does can be found [here](https://github.com/osamc-lv2-workshop/lv2-workshop). Possible additional requirements may include [xmlstarlet](http://xmlstar.sourceforge.net/), [xxd](https://linux.die.net/man/1/xxd) and of course [csound](https://github.com/csound/csound)

## Basic Idea

All the cool stuff shouold be doable in the csd files, so parameters of the plugin and details of how to route midi input to it and other things can be described in an xml file. The Makefile generates all the boilerplated code from xsl templates.

## Intended workflow

To make a new plugin: 

duplicate `plugins/SimpleSynth` to a new directory of your choosing, for instance `plugins/BetterSimpleSynth`

edit `orc.csd` and `plugin.xml`

run `make` from the root dir of simplesynth

copy or link the resulting `.lv2` directory in `bin` to your lv2 plugins directory