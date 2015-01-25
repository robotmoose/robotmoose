INC=../include
CXX=g++
CFLAGS=-O -std=c++11 -Wall -I$(INC)
LIBS=-lpthread -lopencv_core -lopencv_highgui -luriparser

json_src=$(INC)/json.cpp
mongoose_src=$(INC)/mongoose/mongoose.c
msl_src=$(INC)/msl/socket.cpp $(INC)/msl/time.cpp $(INC)/msl/webserver.cpp
webcam_src=webcam.cpp

all: server client

server: server.cpp
	$(CXX) $(CFLAGS) $< $(json_src) $(mongoose_src) $(msl_src) $(webcam_src) -o $@ $(LIBS)

client: client.cpp
	$(CXX) $(CFLAGS) $< $(json_src) $(mongoose_src) $(msl_src) $(webcam_src) -o $@ $(LIBS)

install_ubuntu:
	-sudo apt-get install libopencv-dev liburiparser-dev

clean:
	-rm -f server
	-rm -f client
