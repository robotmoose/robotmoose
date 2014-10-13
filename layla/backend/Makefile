CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -std=c++11
LIBS=-lglut -lGL -lpthread

all: backend

backend: backend.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm backend


# Windows make:
#  cl superstar.cpp mongoose/mongoose.c /I. /Imongoose /EHsc /MT

