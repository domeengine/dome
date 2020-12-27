test.dylib: test.c
	gcc -dynamiclib -o test.dylib -I../../include test.c -undefined dynamic_lookup

test.so:
	gcc -shared -o test.so -fPIC  -I../../include test.c
