INC=../../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

arduino_src=$(INC)/arduino.cpp
json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/serial.cpp $(INC)/msl/string.cpp $(INC)/msl/time.cpp
littlestar_src=littlestar.cpp

all: quizbot

quizbot: main.cpp
	$(CXX) $(CFLAGS) $< $(arduino_src) $(json_src) $(mongoose_src) $(msl_src) $(littlestar_src) -o $@ $(LIBS)

clean:
	-rm -f quizbot
