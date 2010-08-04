CC=`fltk-config --cc`
CFLAGS=-Wall -O2
CXX=`fltk-config --cxx`
#CXXFLAGS=-g -Wall
CXXFLAGS=`fltk-config --cxxflags` `fltk-config --optim`
LDFLAGS=`fltk-config --ldflags`

all: rpbar rpbarsend

rpbar.o: rpbar.cc rpbar.hh settings.hh

rpbar: rpbar.o

rpbarsend.o: rpbarsend.c

rpbarsend: rpbarsend.o

clean:
	rm *.o rpbar rpbarsend

.PHONY: all clean
