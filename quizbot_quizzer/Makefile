INC=../include
CXX=g++-4.8
CFLAGS=-O -pthread -std=c++11 -Wall -I$(INC) -m32
LIBS=

arduino_src=$(INC)/arduino.cpp
msl_src=$(INC)/msl/serial.cpp $(INC)/msl/string.cpp $(INC)/msl/time.cpp
reader_src=reader.cpp

all: quizzer

quizzer: main.cpp
	$(CXX) $(CFLAGS) $< $(arduino_src) $(msl_src) $(reader_src) -o $@ $(LIBS)

clean:
	-rm -f quizzer
