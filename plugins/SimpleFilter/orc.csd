turnon 1

instr 1
ainL, ainR 			ins
kRoomSize 			chnget "RoomSize"
kHFDamp 			chnget "HFDamp"
aoutL, aoutR 		freeverb ainL, ainR, kRoomSize, kHFDamp
					outs aoutL, aoutR
endin
