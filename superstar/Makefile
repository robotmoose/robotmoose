SRC=src
INC=include
CXX=g++
CFLAGS=-O -Wall -Wno-unused-result -I$(INC) -I$(SRC)
LIB=

JSONCPP_SRC=$(INC)/jsoncpp/json_reader.cpp $(INC)/jsoncpp/json_value.cpp $(INC)/jsoncpp/json_writer.cpp
MONGOOSE_SRC=$(INC)/mongoose/mongoose.c
UTILITY_SRC=$(SRC)/json_util.cpp $(SRC)/mongoose_util.cpp $(SRC)/string_util.cpp
COMMON_SRC=$(SRC)/auth.cpp $(SRC)/comet.cpp $(SRC)/jsonrpc.cpp $(SRC)/superstar.cpp $(SRC)/time_util.cpp $(SRC)/web_util.cpp

ifeq ($(OS),Windows_NT)
	LIB+=-lWs2_32
endif

ifeq ($(shell uname),Darwin)
	CFLAGS+=-Wno-deprecated-register
endif

all: superstar

superstar: $(SRC)/main.cpp $(COMMON_SRC) $(JSONCPP_SRC) $(MONGOOSE_SRC) $(UTILITY_SRC)
	$(CXX) $(CFLAGS) $^ $(LIB) -o $@

clean:
	-rm -f superstar superstar.exe