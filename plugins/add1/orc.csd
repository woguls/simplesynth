giSine    ftgen     0, 0, 2^10, 10, 1
; Generate two empty tables for adsynt.
gifrqs ftgen 2, 0, 128, 7, 0, 32, 0
; A table for freqency and amp parameters.
giamps ftgen 3, 0, 128, 7, 0, 32, 0
  

; Generates parameters every k-cycle.
instr 1 
  ichnl   =         1

  ; Generate 10 voices.
  icnt_ctrl chnget "voices"
  icnt = icnt_ctrl + 1
  ; Reset loop index.
  kindex = 0

  ; midi channel for ctrl7
  kspeedmod chnget "speedmod" ; ctrl7 ichnl, 71, 1, 127
  kdepthpow chnget "depthpow" ;, 76, 0.8, 2
  kfreqpow chnget "freqpow" ;, 77, 1.0, 3
  klfoscale chnget "lfoscale"; , 93, 0.006, 0.06
  kVol    chnget "volume" ;     ichnl,74,0,1

  krel init 0
  krel release ;outputs release-stage flag (0 or 1 values)
  if (krel == 1) kgoto loop ;if in release-stage don't send control signals


; Loop executed every k-cycle.
loop:
  ; Generate lfo for frequencies.
  kspeed  pow kindex + 1, 1.6
  ; Individual phase for each voice.



  
  kphas phasorbnk kspeed * kspeedmod * (1/127), kindex, icnt
  klfo table kphas, giSine, 1
  ; Arbitrary parameter twiddling...
  kdepth pow kdepthpow, kindex
  kfreq pow kindex + 1, kfreqpow
  kfreq = kfreq + klfo*klfoscale*kdepth

  ; Write freqs to table for adsynt.
  tablew kfreq, kindex, gifrqs 
  
  ; Generate lfo for amplitudes.
  kspeed  pow kindex + 1, 0.8
  ; Individual phase for each voice.
  kphas phasorbnk kspeed*0.13, kindex, icnt, 2
  klfo table kphas, giSine, 1
  ; Arbitrary parameter twiddling...
  kamp pow 1 / (kindex + 1), 0.4
  kamp = kamp * (0.3+0.35*(klfo+1))

  ; Write amps to table for adsynt.
  tablew kamp, kindex, giamps
  
  kindex = kindex + 1
  ; Do loop.
  if (kindex < icnt) kgoto loop

  iCps = cpsmidinn(p5)
  iAmp = p4/127 * 0dbfs * 0.3
  kenv	madsr	  0.5, 0, 1, 0.5

  asig adsynt kVol*iAmp, iCps, giSine, gifrqs, giamps, icnt
  outs kenv*asig, kenv*asig
endin