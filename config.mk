PREFIX = /usr/local

INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

CFLAGS = -O0 -g -Wall -Wextra -Werror -pedantic -std=c99 ${INCS}
LDFLAGS = ${LIBS}

CC = cc
