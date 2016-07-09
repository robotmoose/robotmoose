SRC=src
INC=include
CXX=g++
CFLAGS=-O -Wall -Wno-unused-result -I$(INC) -I$(SRC)
LIB=

JSONCPP_SRC=$(INC)/jsoncpp/json_reader.cpp $(INC)/jsoncpp/json_value.cpp $(INC)/jsoncpp/json_writer.cpp
MONGOOSE_SRC=$(INC)/mongoose/mongoose.c
UTILITY_SRC=$(SRC)/json_util.cpp $(SRC)/string_util.cpp

ifeq ($(OS),Windows_NT)
	LIB+=-lWs2_32
endif

ifeq ($(shell uname),Darwin)
	CFLAGS+=-Wno-deprecated-register
endif

all: superstar

superstar: $(SRC)/jsonrpc.cpp $(SRC)/main.cpp $(SRC)/superstar.cpp $(JSONCPP_SRC) $(MONGOOSE_SRC) $(UTILITY_SRC)
	$(CXX) $(CFLAGS) $^ $(LIB) -o $@

clean:
	-rm -f superstar superstar.exe