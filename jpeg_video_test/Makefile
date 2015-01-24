INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread -ljpeg -lopencv_core -lopencv_highgui

jpeg_src=jpeg.cpp
json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/webserver.cpp

all: test

test: main.cpp
	$(CXX) $(CFLAGS) $< $(jpeg_src) $(json_src) $(mongoose_src) $(msl_src) -o $@ $(LIBS)

clean:
	-rm -f test
