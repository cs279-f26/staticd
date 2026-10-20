CC      ?= cc
CFLAGS  ?= -O2 -g -Wall -Wextra
LDFLAGS ?=

all: staticd

staticd: src/main.c src/http.c src/http.h
	$(CC) $(CFLAGS) -o $@ src/main.c src/http.c $(LDFLAGS)

test: staticd
	sh tests/run.sh ./staticd

clean:
	rm -f staticd

.PHONY: all test clean
