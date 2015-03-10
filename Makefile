CC=gcc
IDIR=src
OPT:=-Wall -pedantic -std=c99 -g

all: test release

release: src/exosite.c
	$(CC) $(OPT) -Os -c src/exosite.c \
	    -I./pal/template \
	    -I./picocoap/src \
	    ./picocoap/picocoap.o \
	     -o exosite.o

debug: src/exosite.c
	$(CC) $(OPT) -c src/exosite.c -o exosite.o


test: tests/test.c
	$(CC) $(OPT) tests/test.c \
		     tests/cmocka/src/cmocka.c \
		-Itests/cmocka/include \
		-Itests \
		-D_GNU_SOURCE \
		-DHAVE_SIGNAL_H \
		-o test
	./test
	rm test

posixsubscribe: picocoap
	$(CC) $(OPT) examples/subscribe.c \
	             src/exosite.c \
	             pal/posix/exosite_pal.c \
	             picocoap/picocoap.o \
	    -D_POSIX_C_SOURCE=200112L \
	    -Isrc \
	    -Ipal/posix \
	    -Ipicocoap/src \
	    -o posixsubscribe

picocoap:
	$(MAKE) -C picocoap

.PHONY: picocoap test

clean:
	rm -f test
	rm -f posixclient
	rm -f posixsubscribe
	rm -rf *.dSYM
