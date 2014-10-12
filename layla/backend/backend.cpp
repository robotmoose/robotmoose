/**
  The backend reads HTTP commands from superstar, and writes serial
  commands to the arduino.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-10-11 (public domain)
*/
#include "../firmware/robot.h" // robot structs
#include "../firmware/serial_packet.h" // serial comms format
#include "../../include/serial.h" // serial port access

#include "../../include/osl/time_function.h" // timing
#include "../../include/osl/webservice.h" // web client

/* Do linking right here */
#include "../../include/osl/webservice.cpp"
#include "../../include/osl/socket.cpp"
#include "../../include/serial.cpp"


int main(int argc,char *argv[]) {
	// Superstar path:
	std::string URL="http://sandy.cs.uaf.edu/";
	std::string robotName="layla/uaf";
	if (argc>1) robotName=argv[1];
	if (argc>2) URL=argv[2];
	
	double program_start=time_in_seconds();
	osl::url_parser pu(URL);
	osl::http_connection superstar(pu.host,0,pu.port);
	
	while (1) {
		std::string path="/superstar/"+robotName+"/pilot?get";
		double start=time_in_seconds();
		printf("Path %s\n",path.c_str());
		superstar.send_get(path);
		std::string data=superstar.receive();
		
		printf("Got %ld bytes of data: %s\n",data.size(),data.c_str());
		double elapsed=time_in_seconds()-start;
		double per=elapsed;
		printf("	%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);	
	}
}

