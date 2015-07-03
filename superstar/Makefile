CXX=g++
OPTS=-O
CFLAGS=$(OPTS) -Wall -I../include
# CFLAGS+=-DLISTEN80
LIBS=-lpthread

all: superstar netbench

superstar: superstar.cpp
	$(CXX) $(CFLAGS) $< mongoose/mongoose.c -o $@ $(LIBS)

# This is needed to allow access to port 80
supercap: superstar
	sudo setcap 'cap_net_bind_service=+ep' superstar

netbench: netbench.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

netbench_keepalive: netbench_keepalive.cpp
	$(CXX) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	- rm -f superstar


# Windows make:
#  cl superstar.cpp mongoose/mongoose.c /I. /Imongoose /EHsc /MT

