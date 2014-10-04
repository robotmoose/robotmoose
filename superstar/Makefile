CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -I. -Imongoose -I../include -Wall
LIBS=-lpthread

all: superstar netbench

superstar: superstar.cpp
	$(CXX) $(CFLAGS) $< mongoose/mongoose.c -o $@ $(LIBS)

netbench: netbench.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm superstar


# Windows make:
#  cl superstar.cpp mongoose/mongoose.c /I. /Imongoose /EHsc /MT

