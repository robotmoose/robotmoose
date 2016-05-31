CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -lGL -lGLU -lglut -lGLEW -std=c++1y
LIBS=

ifeq ($(OS),Windows_NT)
	LIBS+=-lWs2_32
else

	ifeq ($(shell uname),Darwin)
		LIBS+=-framework IOKit -framework CoreFoundation 
	endif
endif

all: cartogrobot

cartogrobot: cartogrobot.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -rf cartogrobot cartogrobot.exe
