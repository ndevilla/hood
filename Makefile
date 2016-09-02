
CFLAGS = -O3 -g -I. -Wall -pedantic $(EXTRA)

all: hood

hood: hood.c hood.h
	$(CC) $(CFLAGS) -o $@ $< -DMAIN

clean:
	rm -f hood
	find . \( -name \*.dSYM  -prune \) -exec rm -rf '{}' ';'
