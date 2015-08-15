CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -ggdb
LIBS=

ifeq ($(OS),Windows_NT)
	LIBS+=-lWs2_32
else

	ifeq ($(shell uname),Darwin)
		LIBS+=-framework IOKit -framework CoreFoundation 
	endif
endif

all: backend

backend: backend.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -rf backend backend.exe
