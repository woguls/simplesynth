instr 1
iamp = p4/127
ipch = cpsmidinn(p5)
kvol init 0.5
kvol chnget "paramVolume"
a1   poscil iamp, ipch
     outs    kvol*a1, kvol*a1
endin
