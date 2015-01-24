INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread -lopencv_core -lopencv_highgui

json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/time.cpp $(INC)/msl/webserver.cpp
webcam_src=webcam.cpp

all: test

test: main.cpp
	$(CXX) $(CFLAGS) $< $(json_src) $(mongoose_src) $(msl_src) $(webcam_src) -o $@ $(LIBS)

clean:
	-rm -f test
