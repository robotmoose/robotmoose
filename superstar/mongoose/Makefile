CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -I. -Wall
LIBS=-lpthread

all: demo

demo: demo.cpp
	$(CXX) $(CFLAGS) demo.cpp mongoose.c -I. -o demo $(LIBS)

clean:
	- rm demo
