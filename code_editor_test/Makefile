INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/time.cpp $(INC)/msl/webserver.cpp

all: code_editor

code_editor: main.cpp
	$(CXX) $(CFLAGS) $< $(json_src) $(mongoose_src) $(msl_src) -o $@ $(LIBS)

clean:
	-rm -f code_editor
