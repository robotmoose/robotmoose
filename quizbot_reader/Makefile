INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

arduino_src=$(INC)/arduino.cpp
msl_src=$(INC)/msl/serial.cpp $(INC)/msl/string.cpp $(INC)/msl/time.cpp
reader_src=reader.cpp

all: quizbot_reader

quizbot_reader: main.cpp
	$(CXX) $(CFLAGS) $< $(arduino_src) $(msl_src) $(reader_src) -o $@ $(LIBS)

clean:
	-rm -f quizbot_reader
