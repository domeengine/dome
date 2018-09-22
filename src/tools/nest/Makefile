src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS = -std=c99 -pedantic -Wall
LDFLAGS = -lm

nest: *.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) nest
