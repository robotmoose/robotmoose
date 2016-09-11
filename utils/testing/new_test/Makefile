SRC=src
INC=../../../include
CXX=g++
CFLAGS=-O -Wall -Wno-unused-result -I$(INC) -I$(SRC) -std=c++11 -DMG_ENABLE_SSL
LIB=

JSONCPP_SRC=$(INC)/jsoncpp/json_reader.cpp $(INC)/jsoncpp/json_value.cpp $(INC)/jsoncpp/json_writer.cpp
MONGOOSE_SRC=$(INC)/mongoose/mongoose.c
UTILITY_SRC=$(INC)/auth.cpp $(INC)/json_util.cpp $(INC)/superstar.cpp $(INC)/string_util.cpp $(INC)/time_util.cpp $(INC)/wget.cpp

ifeq ($(OS),Windows_NT)
	LIB+=-lWs2_32
else
	LIB+=-lcrypto -lssl
endif

ifeq ($(shell uname),Darwin)
	CFLAGS+=-Wno-deprecated-register
endif

all: benchmark

benchmark: $(SRC)/main.cpp $(COMMON_SRC) $(JSONCPP_SRC) $(MONGOOSE_SRC) $(UTILITY_SRC)
	$(CXX) $^ $(LIB) $(CFLAGS) -o $@

clean:
	-rm -f benchmark benchmark.exe
