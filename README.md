# Simple Synth

A simple way to make lv2 plugins with csound

This project uses this [distrho plugin framework](https://github.com/DISTRHO/DPF/)

Build instructions and a better explanation of what the code does can be found [here](https://github.com/osamc-lv2-workshop/lv2-workshop)

## Basic Idea

All the cool stuff shouold be doable in the csd files, so parameters of the plugin and details of how to route midi input to it and other things can be described in an xml file. The Makefile generates all the boilerplated code from xsl templates.
