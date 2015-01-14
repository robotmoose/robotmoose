INC=../../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/serial.cpp $(INC)/msl/string.cpp $(INC)/msl/time.cpp

all: quizbot

quizbot: quizbot.cpp
	$(CXX) $(CFLAGS) $< $(mongoose_src) $(msl_src) -o $@ $(LIBS)

clean:
	- rm quizbot