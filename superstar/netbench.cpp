/**
  Repeatedly http fetch a web page, timing the process.
  
  Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2014-10-03 (Public Domain)
*/
#include <iostream>
#include <fstream>
#include "../include/osl/time_function.h"
#include "../include/osl/webservice.h"

/* Do linking right here */
#include "../include/osl/webservice.cpp"
#include "../include/osl/socket.cpp"


int main(int argc,char *argv[]) {
	int count=1;
	std::string URL="http://localhost:8080/";
	if (argc>1) URL=argv[1];
	if (argc>2) count=atoi(argv[2]);
	
	double start=time_in_seconds();
	for (int rep=0;rep<count;rep++) {
		std::string data=osl::download_url(URL);
		if (rep==0) printf("Got %d bytes of data: %s\n",(int)data.size(),data.c_str());
	}
	double elapsed=time_in_seconds()-start;
	double per=elapsed/count;
	printf("%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);
	
	return 0;
}
