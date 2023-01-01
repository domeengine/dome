.PHONY: rs.gen.binds rs.build test.dylib test.so test.dll build.macos build.win build.linux

DOME=../../dome

rs.build rsb:
	cargo build --release
	mv target/release/libexample.a ./libexample.a

rs.gen.binds rsgb:
	cbindgen --config cbindgen.toml --lang c --crate example --output libexample.h

# MacOS
test.dylib: test.c
	gcc -dynamiclib -o test.dylib -I../../include test.c libexample.a -undefined dynamic_lookup

# Linux
test.so: test.c
	gcc -O3 -std=c11 -shared -o test.so libexample.a -fPIC -I../../include test.c

# Windows
test.dll: test.c
	gcc -O3 -std=gnu11 -shared -fPIC  -I../../include test.c libexample.a -Wl,--unresolved-symbols=ignore-in-object-files -o test.dll

build.macos:
	make rs.build
	make rs.gen.binds
	make test.dylib

build.win:
	make rs.build
	make rs.gen.binds
	make test.dll

build.linux:
	make rs.build
	make rs.gen.binds
	make test.so

run:
	${DOME} main.wren
