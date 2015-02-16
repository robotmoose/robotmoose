INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

msl_src=$(INC)/msl/serial.cpp $(INC)/msl/time.cpp
roomba_src=arduino/roomba.cpp

all: roomba

roomba: main.cpp
	$(CXX) $(CFLAGS) $< $(arduino_src) $(msl_src) $(roomba_src) -o $@ $(LIBS)

clean:
	-rm -f roomba
