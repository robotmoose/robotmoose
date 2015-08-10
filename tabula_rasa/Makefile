CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -ggdb
LIBS=-lpthread

ifeq ($(shell uname),Darwin)
	LIBS+=-framework IOKit -framework CoreFoundation 
endif

all: backend

backend: backend.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -rf backend

