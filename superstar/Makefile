CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -I. -Imongoose -Wall
LIBS=-lpthread

all: superstar

superstar: superstar.cpp
	$(CXX) $(CFLAGS) $< mongoose/mongoose.c -I. -o $@ $(LIBS)

clean:
	- rm superstar


# Windows make:
#  cl superstar.cpp mongoose/mongoose.c /I. /Imongoose /EHsc /MT

