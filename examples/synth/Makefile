.PHONY: synth.dylib
synth.dylib: synth.c
	gcc -dynamiclib -o synth.dylib -I../../include synth.c -undefined dynamic_lookup

synth.so:
	gcc -shared -o synth.so -fPIC  -I../../include synth.c
