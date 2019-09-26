/**
  Repeatedly http fetch a web page, timing the process.
  Uses HTTP keepalive, to reuse the same socket over and over.
  
  Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2014-10-03 (Public Domain)
*/
#include <iostream>
#include <fstream>
#include "osl/time_function.h"
#include "osl/webservice.h"

/* Do linking right here */
#include "osl/webservice.cpp"
#include "osl/socket.cpp"


int main(int argc,char *argv[]) {
	int count=1;
	std::string URL="http://localhost:8080/";
	if (argc>1) URL=argv[1];
	if (argc>2) count=atoi(argv[2]);
	
	double start=time_in_seconds();
	osl::url_parser pu(URL);
	osl::http_connection c(pu.host,0,pu.port);

	for (int rep=0;rep<count;rep++) {
		c.send_get(pu.path);
		std::string data=c.receive();
		
		if (rep==0) printf("Got %ld bytes of data: %s\n",data.size(),data.c_str());
	}
	double elapsed=time_in_seconds()-start;
	double per=elapsed/count;
	printf("%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);
	
	return 0;
}
