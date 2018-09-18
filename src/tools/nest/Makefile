src = $(wildcard *.c)
obj = $(src:.c=.o)

# LDFLAGS = -lm

nest: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) nest
