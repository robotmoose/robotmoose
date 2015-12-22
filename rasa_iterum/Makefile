CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -Wno-tautological-constant-out-of-range-compare
LIBS=
INC=../include/

ifeq ($(OS),Windows_NT)
	LIBS+=-lWs2_32
else

	ifeq ($(shell uname),Darwin)
		LIBS+=-framework IOKit -framework CoreFoundation
	endif
endif

all: rasa_iterum

rasa_iterum: rasa_iterum.cpp $(INC)/serial.cpp $(INC)/mongoose/mongoose.c $(INC)/json.cpp
	$(CXX) $(CFLAGS) -I$(INC) $^ -o $@ $(LIBS)

clean:
	- rm -rf rasa_iterum rasa_iterum.exe
