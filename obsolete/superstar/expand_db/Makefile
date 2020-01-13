all: expand_db

expand_db: expand_db.cpp
	g++ -std=c++11 -O2 $^ -o $@

clean:
	-rm -f expand_db expand_db.exe
