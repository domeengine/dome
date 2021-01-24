.PHONY: test.dylib
test.dylib: test.c
	gcc -dynamiclib -o test.dylib -I../../include test.c -undefined dynamic_lookup

test.so: test.c
	gcc -O3 -std=c11 -shared -o test.so -fPIC  -I../../include test.c

test.dll: test.c
	gcc -O3 -std=gnu11 -shared -fPIC  -I../../include test.c -Wl,--unresolved-symbols=ignore-in-object-files -o test.dll
