/**
  The backend reads HTTP pilot commands from superstar, writes serial
  commands to the arduino, and sends sensor data back up.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-21 (public domain)
*/
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

// Arduino comms:
#include "arduino/serial_packet.h" // serial comms format
#include "arduino/tabula_control.h" // sensor and command formatting
#include "../include/serial.h" // serial port access

// Network comms:
#include "../include/osl/time_function.h" // timing
#include "../include/osl/webservice.h" // web client
#include "../include/json.h" // JSON parsing ("super easy JSON" library)

// Script execution
#ifndef	_WIN32
#include <unistd.h> /* for fork(), execl() */
#endif

/* Do linking right here */
#include "../include/serial.cpp"
#include "../include/osl/socket.cpp"
#include "../include/osl/webservice.cpp"
#include "../include/json.cpp"


/**
  Read commands from superstar, and send them to the robot.
*/
class robot_backend {
private:
	osl::url_parser parseURL;
	osl::http_connection superstar; // HTTP keepalive connection
	std::string robotName;
	A_packet_formatter<SerialPort> *pkt;
public:
	double LRtrim;
	bool debug;

	robot_backend(std::string superstarURL, std::string robotName_)
		:parseURL(superstarURL), superstar(parseURL.host,0,parseURL.port),
		robotName(robotName_), pkt(0), LRtrim(1.0)
	{
		//stop();
	}
	~robot_backend() {
		delete pkt;
	}
	void read_sensors(const A_packet& current_p);
	
	/** Update pilot commands from network */
	void read_network(void);
	void send_network(void);
	
	// Raw binary data from dynamically configured Arduino devices:
	std::vector<unsigned char> sensor;
	std::vector<unsigned char> command;
	
	// Configure these devices
	void setup_devices(std::string robot_config);
	
	/** Talk to this real Arudino device over this serial port.
	    Run this robot configuration. */
	void setup_arduino(SerialPort &port,std::string robot_config);
	
	/** Send pilot commands to robot. */
	void send_serial();
	/** Read anything the robot wants to send to us. */
	void read_serial(void);
};

// Read a line of ASCII from this serial port.
std::string getline_serial(SerialPort &port) {
	std::string ret="";
	while (true) {
		int c=Serial.read();
		if (c==-1) continue; // busy wait
		else if (c=='\r' || c=='\n' || c>=0x80) {
			if (ret!="") return ret;
		}
		else ret+=c;
	}
}

void robot_backend::setup_devices(std::string robot_config)
{
	// Parse lines of the configuration outselves, to 
	//   find the command and sensor fields and match them to JSON
	std::istringstream robot_config_stream( robot_config );
	while (robot_config_stream) {
		std::string device=""; robot_config_stream>>device;
		std::string pins_etc="";
		if (!std::getline(robot_config_stream,pins_etc)) break;
		
		
		// This is probably stupid, and should be replaced with registration from tabula_config.h
		if (device=="serial_controller") { 
			break; // end of file
		}
		else if (device=="cmd") {
			continue; // skip configuration commands
		}
		else if (device=="analog_sensor") {
			// make_sensor<unsigned int>
		}
		else if (device=="pwm_pin") {
			// make_command<unsigned char>
			command.push_back(200);
		}
		else std::cout<<"Arduino backend: ignoring unknown device '"<<device<<"'\n";
	}
}

void robot_backend::setup_arduino(SerialPort &port,std::string robot_config)
{
	std::string start=getline_serial(port); // wait for Arduino to boot
	std::cout<<"Arduino startup: "<<start<<"\n";
	port.write(&robot_config[0],robot_config.size()); // dump to Arduino
	while (true) {
		std::string status=getline_serial(port);
		if (status=="-1") break;
		std::cout<<"Arduino status: "<<status<<"\n";
	}
	
	std::cout<<"Arduino switching to binary communication\n";
	delete pkt; pkt=0;
	pkt=new A_packet_formatter<SerialPort>(port);
	send_serial();
	read_serial();
}

void robot_backend::read_sensors(const A_packet& current_p)
{
	printf("Arduino sent %d bytes of sensor data\n",current_p.length);
	// if (current_p.length!=
	// memcpy(&sensors[0],current_p.data,current_p.length);
}

/** Read anything the robot wants to send to us. */
void robot_backend::read_serial(void) {
	if (pkt==0) return; // simulation only
	while (Serial.available()) { // read any robot response
		if (debug) printf("v");
		int16_t got_data=0;
		A_packet p;
		while (-1==pkt->read_packet(p)) { }
		if (p.valid) {
			if (debug) printf("P");
			printf("Arduino sent packet type %x (%d bytes):\n",p.command,got_data); // last printf give all of this (no longer need)
			if (p.command == 0) printf("    Arduino sent echo request %d bytes, '%.*s'\n", p.length, p.length, p.data);
			else if (p.command==0xE) printf("    Arduino sent error packet: %d bytes, '%.*s'\n", p.length, p.length, p.data);
			else if (p.command == 0xC)
			{
				//printf("    Arduino sent sensor data: %d bytes, '%.*s'\n", p.length, p.length, p.data);
				read_sensors(p);
			}
			else printf("    Arduino sent unknown packet 0x%X command %d bytes, '%.*s'\n", p.command, p.length, p.length, p.data);
		}
	}
}

/** Send data to the robot over serial connection */
void robot_backend::send_serial(void) {
	if (pkt==0) return; // simulation only
	
	pkt->write_packet(0xC,command.size(),&command[0]);
}

/** Read pilot data from superstar, and store into ourselves */
void robot_backend::read_network()
{
	std::string path="/superstar/"+robotName+"/pilot?get";
	double start=time_in_seconds();
	superstar.send_get(path);
	std::string json_data=superstar.receive();

/*
	try {
		json::Value v=json::Deserialize(json_data);
		L=limit_power(v["power"]["L"]);
		R=limit_power(LRtrim*float(v["power"]["R"]));
		S1 = v["power"]["arms"];
		S2 = v["power"]["mine"];
		S3 = v["power"]["dump"];
		ledOn = v["LED"]["On"];
		ledDemo = v["LED"]["Demo"];
		double tempRBG = 0;
		tempRBG = v["LED"]["R"];
		led_red = (int16_t)(tempRBG * 255);
		tempRBG = v["LED"]["G"];
		led_green = (int16_t)(tempRBG * 255);
		tempRBG = v["LED"]["B"];
		led_blue = (int16_t)(tempRBG * 255);
#ifndef	_WIN32
		static std::string last_cmd_arg="";
		std::string run=v["cmd"]["run"];
		std::string arg=v["cmd"]["arg"];
		if (run.find_first_of("./\\\"")==std::string::npos) { // looks clean
			std::string cmd_arg=run+arg;
			if (last_cmd_arg!=cmd_arg) { // new script command: run it
				std::string path="./"+run;
				printf("RUNNING SCRIPT: '%s' with arg '%s'\n",
					path.c_str(),arg.c_str());
				if (fork()==0) {
					if (chdir("scripts")!=0) {
						printf("SCRIPT chdir FAILED\n");
					}
					else {
						execl(path.c_str(),path.c_str(),arg.c_str(),(char *)NULL);
						perror("SCRIPT EXECUTE FAILED\n");
					}
					exit(0);
				}
			}
			last_cmd_arg=cmd_arg;
		}
#endif
		if (debug)
		{
			printf("Power commands: %.2f L  %.2f R  %.2f S1  %.2f S2 %.2f S3\nLED on/demo : ", L, R, S1, S2, S3);
			printf(ledOn ? "true/" : "false/");
			printf(ledDemo ? "true\n" : "false\n");
			printf("RGB Values: %d R %d G %d B \n", led_red, led_blue, led_green);
			//printf("   Network data: %ld bytes, '%s'\n", json_data.size(), json_data.c_str());
		}
	} catch (std::exception &e) {
		printf("Exception while processing network JSON: %s\n",e.what());
		printf("   Network data: %ld bytes, '%s'\n", json_data.size(),json_data.c_str());
		stop();
	}
*/
	double elapsed=time_in_seconds()-start;
	double per=elapsed;
	printf("\nRead Time:	%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);
}

void robot_backend::send_network(void)
{
	double start = time_in_seconds();
/*
	std::string path = "/superstar/" + robotName + "/data?set="; //data from robot
	//uggly will fix this so that its not converting so much
	json::Object temp;
	temp["uSound1"] = current_sensors.uSound1;
	temp["uSound2"] = current_sensors.uSound2;
	temp["uSound3"] = current_sensors.uSound3;
	temp["uSound4"] = current_sensors.uSound4;
	temp["uSound5"] = current_sensors.uSound5;
	//end uggly
	std::string data = json::Serialize(temp);
	//auth!!!! grrr
	superstar.send_get(path+data); //auth will fail this
	// above needs fixed!!! but its not braking anything right now just not setting

*/
	double elapsed = time_in_seconds() - start;
	double per = elapsed;
	printf("Send Time:	%.1f ms/request, %.1f req/sec\n", per*1.0e3, 1.0 / per);
}

robot_backend *backend=NULL; // the singleton robot

int main(int argc, char *argv[])
{
	double LRtrim=1.0;
	bool sim = false; // use --sim to enable simulation mode
	bool debug = false;  // spams more output data
	std::string superstarURL = "http://sandy.cs.uaf.edu/";
	std::string robotName = "layla/uaf";
	int baudrate = 57600;  // serial comms
	for (int argi = 1; argi<argc; argi++) {
		if (0 == strcmp(argv[argi], "--robot")) robotName = argv[++argi];
		else if (0 == strcmp(argv[argi], "--superstar")) superstarURL = argv[++argi];
		else if (0 == strcmp(argv[argi], "--baudrate")) baudrate = atoi(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--trim")) LRtrim = atof(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--debug")) debug = true;
		else if (0 == strcmp(argv[argi], "--sim")) { // no hardware, for debugging
			robotName="sim/uaf";
			sim = true;
			baudrate=0;
		}
		else {
			printf("Unrecognized command line argument '%s'\n", argv[argi]);
		}
	}
	backend=new robot_backend(superstarURL, robotName);
	backend->LRtrim=LRtrim;
	backend->debug = debug; // more output, more mess, but more data
	
	
	std::string robot_config=
"analog_sensor A0\n"
"analog_sensor A5\n"
"pwm_pin 3\n"
"cmd 0 200\n"
"serial_controller\n"
;
	backend->setup_devices(robot_config);
	
	if (!sim) {
		Serial.begin(baudrate);
		backend->setup_arduino(Serial,robot_config);
	}

	while (1) { // talk to robot via backend
		backend->read_network();
		backend->send_serial();
		backend->read_serial();
		backend->send_network();
#ifdef __unix__
		usleep(10*1000); // limit rate to 100Hz, to be kind to serial port and network
#endif
	}

	return 0;
}


