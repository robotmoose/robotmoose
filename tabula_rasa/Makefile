CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -ggdb
LIBS=-lpthread

all: backend

backend: backend.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -rf backend

