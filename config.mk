PREFIX = /usr/local

INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

CFLAGS = -Wall -pedantic -O0 -g ${INCS}
LDFLAGS = ${LIBS}

CC = cc
