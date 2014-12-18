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

test: tests/test.c src/exosite.h
	$(CC) $(OPT) tests/test.c \
	             src/exosite.c \
	             pal/template/exosite_pal.c \
	             picocoap/picocoap.o \
	    -Ipal/template \
	    -Ipicocoap/src \
	    -o test
	./test
	rm test

buildtest: tests/test.c src/exosite.h
	$(CC) $(OPT) tests/test.c \
	             src/exosite.c \
	             pal/template/exosite_pal.c \
	    -Ipal/template \
	    -Ipicocoap/src \
	    -o test

posixclient: 
	$(CC) $(OPT) examples/polling_read_write.c \
	             src/exosite.c \
	             pal/posix/exosite_pal.c \
	             picocoap/picocoap.o \
	    -I./src \
	    -I./pal/posix \
	    -I./picocoap/src \
	    -o posixclient

posixsubscribe: 
	$(CC) $(OPT) examples/subscribe.c \
	             src/exosite.c \
	             pal/posix/exosite_pal.c \
	             picocoap/picocoap.o \
	    -I./src \
	    -I./pal/posix \
	    -I./picocoap/src \
	    -o posixsubscribe

#posixclientd: examples/posix/client_dtls.c src/coap.h
#	$(CC) $(OPT) -D_POSIX_SOURCE examples/posix/client_dtls.c src/coap.c -o posixclientd

clean:
	rm -f test
	rm -f posixclient
