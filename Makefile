CC=gcc
CXX=g++
CFLAGS=-Os -Wall
CXXFLAGS=-Os -Wall
LIBS=-lX11

BIN=rpbar rpbarsend

all: ${BIN}

.c.o:
	@${CC} -c ${CFLAGS} $<

.cc.o:
	@${CXX} -c ${CXXFLAGS} $<

rpbar.o: rpbar.cc rpbar.hh settings.hh

rpbar: rpbar.o
	${CXX} -o $@ $^ ${LIBS}

rpbarsend.o: rpbarsend.c

rpbarsend: rpbarsend.o settings.hh
	${CC} -o $@ $^ ${LIBS}

clean:
	rm *.o ${BIN}

.PHONY: all clean
