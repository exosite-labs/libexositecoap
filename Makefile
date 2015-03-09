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

test: tests/test.c testframework
	$(CC) $(OPT) tests/test.c \
		     tests/cmocka/build/src/libcmocka.0.dylib \
		-Itests/cmocka/include \
		-Itests/cmocka/build \
		-o test
	./test
	rm test

testframework:
	mkdir -p tests/cmocka/build
	cd tests/cmocka/build && \
	cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug .. && \
	make

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

.PHONY: picocoap

#posixclientd: examples/posix/client_dtls.c src/coap.h
#	$(CC) $(OPT) -D_POSIX_SOURCE examples/posix/client_dtls.c src/coap.c -o posixclientd

clean:
	rm -f test
	rm -f posixclient
	rm -f posixsubscribe
	rm -rf *.dSYM
