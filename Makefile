CXX=g++
CXXFLAGS=-Os -Wall
LIBS=-lX11

BIN=rpbar rpbarsend

all: ${BIN}

.cc.o:
	@${CXX} -c ${CXXFLAGS} $<

rpbar.o: rpbar.cc rpbar.hh settings.hh

rpbar: rpbar.o
	${CXX} -o $@ $^ ${LIBS}

rpbarsend.o: rpbarsend.cc

rpbarsend: rpbarsend.o settings.hh
	${CXX} -o $@ $^ ${LIBS}

clean:
	rm *.o ${BIN}

.PHONY: all clean
