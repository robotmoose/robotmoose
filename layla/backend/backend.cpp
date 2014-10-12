/**
  The backend reads HTTP commands from superstar, and writes serial
  commands to the arduino.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-10-11 (public domain)
*/
#include <stdexcept>

// Arduino comms:
#include "../firmware/robot.h" // robot structs
#include "../firmware/serial_packet.h" // serial comms format
#include "../../include/serial.h" // serial port access

// Network comms:
#include "../../include/osl/time_function.h" // timing
#include "../../include/osl/webservice.h" // web client
#include "../../include/json.h" // JSON parsing ("super easy JSON" library)

/* Do linking right here */
#include "../../include/serial.cpp"
#include "../../include/osl/socket.cpp"
#include "../../include/osl/webservice.cpp"
#include "../../include/json.cpp"


int main(int argc,char *argv[]) {
	// Superstar host name:
	std::string superstarURL="http://sandy.cs.uaf.edu/";
	std::string robotName="layla/uaf";
	int baudrate=9600;  // serial comms
	for (int argi=1;argi<argc;argi++) {
		if (0==strcmp(argv[argi],"--robot")) robotName=argv[++argi];
		else if (0==strcmp(argv[argi],"--superstar")) superstarURL=argv[++argi]; 
		else if (0==strcmp(argv[argi],"--baudrate")) baudrate=atoi(argv[++argi]); 
		else if (0==strcmp(argv[argi],"--sim")) baudrate=0; // no hardware, for debugging 
		else {
			printf("Unrecognized command line argument '%s'\n",argv[argi]);
		}
	}
	SerialPort *serial=0;
	A_packet_formatter<SerialPort> *pkt=0;
	if (baudrate) {
		serial=new SerialPort();
		serial->begin(baudrate);
		pkt=new A_packet_formatter<SerialPort>(*serial);
	}
	
	// double program_start=time_in_seconds();
	osl::url_parser pu(superstarURL);
	osl::http_connection superstar(pu.host,0,pu.port);
	
	while (1) {
		std::string path="/superstar/"+robotName+"/pilot?get";
		double start=time_in_seconds();
		printf("Path %s\n",path.c_str());
		superstar.send_get(path);
		std::string json_data=superstar.receive();
		
		try {
			json::Value v=json::Deserialize(json_data);
			double L=v["power"]["L"];
			double R=v["power"]["R"];
			
			robot_power power;
			power.left=64+64*L;
			power.right=64+64*R;
			printf("Power commands: %.2f L  %.2f R\n",L,R);
			
			if (pkt) {
				pkt->write_packet(0x7,sizeof(power),&power);
				while (Serial.available()) { // read any robot response
					int got_data=0;
					A_packet p;
					while (-1==pkt->read_packet(p)) {got_data++;}
					if (p.valid) {
						printf("Arduino sent packet type %x (%d bytes):\n",p.command,got_data);
						if (p.command==0) printf("    Arduino echo request\n");
						else if (p.command==0xE) printf("    Arduino error packet: %d bytes, '%s'\n", p.length,p.data);
						else printf("    Arduino unknown packet %d bytes\n",p.length);
					}
				}
			}
			
		} catch (std::exception &e) {
			printf("Exception while processing network JSON: %s\n",e.what());
			printf("   Network data: %ld bytes, '%s'\n", json_data.size(),json_data.c_str());
		}
		double elapsed=time_in_seconds()-start;
		double per=elapsed;
		printf("	%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);	
	}
}

