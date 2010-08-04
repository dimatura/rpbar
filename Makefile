CC=`fltk-config --cc`
CFLAGS=-Wall -O2
CXX=`fltk-config --cxx`
#CXXFLAGS=-g -Wall
CXXFLAGS=`fltk-config --cxxflags` `fltk-config --optim`
LDFLAGS=`fltk-config --ldflags`

all: rpbar

rpbar.o: rpbar.cc rpbar.hh settings.hh

rpbar: rpbar.o

clean:
	rm *.o rpbar

.PHONY: all clean
