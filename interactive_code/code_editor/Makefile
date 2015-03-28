INC=../../include
CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -std=c++11 -Wall -I$(INC)
LIBS=-lpthread

cppcheck_src=cppcheck.cpp
json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/string.cpp $(INC)/msl/time.cpp $(INC)/msl/webserver.cpp

all: code_editor

code_editor: main.cpp
	$(CXX) $(CFLAGS) $< $(cppcheck_src) $(json_src) $(mongoose_src) $(msl_src) -o $@ $(LIBS)

clean:
	-rm -f code_editor
