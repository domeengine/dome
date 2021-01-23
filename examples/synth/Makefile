.PHONY: synth.dylib
synth.dylib: synth.c
	gcc -dynamiclib -o synth.dylib -I../../include synth.c -undefined dynamic_lookup -O3

synth.so: synth.c
	gcc -shared -o synth.so -fPIC  -I../../include synth.c -O3

synth.dll: synth.c
	gcc -std=gnu11 -DWIN_EXPORT=1 -shared -fPIC  -I../../include synth.c -lm -latomic -Wl,--unresolved-symbols=ignore-in-object-files -o synth.dll -O3
