INC=../include
C=gcc
CXX=g++
CFLAGS=-c -I$(INC) -Wno-attributes
CXXFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread -lopencv_core -lopencv_highgui

json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/socket.cpp $(INC)/msl/time.cpp $(INC)/msl/webserver.cpp
uri_src=uri.cpp
uri_parser_src=$(INC)/uriparser/*.c
webcam_src=webcam.cpp

all: server forwarder

server: server.cpp
	$(C) $(CFLAGS) $(uri_parser_src)
	$(CXX) $(CXXFLAGS) $< *.o $(json_src) $(mongoose_src) $(msl_src) $(uri_src) $(webcam_src) -o $@ $(LIBS)
	-rm *.o

forwarder: forwarder.cpp
	$(C) $(CFLAGS) $(uri_parser_src)
	$(CXX) $(CXXFLAGS) $< *.o $(json_src) $(mongoose_src) $(msl_src) $(uri_src) $(webcam_src) -o $@ $(LIBS)
	-rm *.o

install_ubuntu:
	-sudo apt-get install libopencv-dev

clean:
	-rm -f server
	-rm -f forwarder
