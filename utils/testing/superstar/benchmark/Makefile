JSONCPP=$(wildcard include/jsoncpp/*.cpp)

run: all
	./superstar_client_test.out

all:
	g++ src/benchmark.cpp include/hash/sha256.cpp $(JSONCPP) -lcurl -o superstar_client_test.out -std=c++14 -O2

clean:
	rm -rf superstar_client_test a.out
