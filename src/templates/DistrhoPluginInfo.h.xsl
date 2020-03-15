<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
<xsl:template match="/">

/*
 * Simple Synth audio efffect based on DISTRHO Plugin Framework (DPF) and Csound
 * Copyright (C) 2018 Christopher Arndt &lt;info@chrisarndt.de&gt;
 * Copyright (C) 2020 David DeWert &lt;daviddewert@gmail.com&gt;

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED &quot;AS IS&quot; AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DISTRHO_PLUGIN_INFO_H
#define DISTRHO_PLUGIN_INFO_H

#define DISTRHO_PLUGIN_BRAND &quot;<xsl:value-of select="/plugin/distrho/brand"/>&quot;
#define DISTRHO_PLUGIN_NAME  &quot;<xsl:value-of select="/plugin/distrho/name"/>&quot;
#define DISTRHO_PLUGIN_URI   &quot;<xsl:value-of select="/plugin/distrho/uri"/>&quot;

#define DISTRHO_PLUGIN_HAS_UI       0
#define DISTRHO_PLUGIN_IS_RT_SAFE   0
#define DISTRHO_PLUGIN_NUM_INPUTS   0
#define DISTRHO_PLUGIN_IS_SYNTH     1
#define DISTRHO_PLUGIN_NUM_OUTPUTS  <xsl:value-of select="/plugin/distrho/outputs"/>
#define DISTRHO_PLUGIN_WANT_TIMEPOS 0
#define DISTRHO_PLUGIN_WANT_PROGRAMS 1
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT  1
#define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 0

#endif // DISTRHO_PLUGIN_INFO_H


</xsl:template>
</xsl:stylesheet>