CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -ggdb
LIBS=-lglut -lGL -lpthread

all: backend

backend: backend.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -rf backend

