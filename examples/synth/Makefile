.PHONY: synth.dylib 

synth.dylib: synth.c
	gcc -dynamiclib -o synth.dylib -I../../include synth.c -undefined dynamic_lookup -O3

synth.so: synth.c
	gcc -O3 -std=c11 -shared -o synth.so -fPIC  -I../../include -lm -latomic synth.c

synth.dll: synth.c
	gcc -O3 -std=gnu11 -shared -fPIC  -I../../include synth.c -lm -latomic -Wl,--unresolved-symbols=ignore-in-object-files -o synth.dll
