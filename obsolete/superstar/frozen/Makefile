CXX=g++
OPT=-O
CFLAGS=$(OPT) -I.
LIBS=

all: test

test: unit_test.c
	$(CXX) $(CFLAGS) $< -I. -o $@ $(LIBS)
	./test

clean:
	- rm test
