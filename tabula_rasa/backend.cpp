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
 This is the location of one field in a JSON object,
   for example, robot.foo.bar[3] stored as {"foo","bar"}, 3
*/
class json_path {
public:
	std::vector<std::string> path;
	int index; // array index, or -1 if not an array
	json_path(const std::string &rootField,int index_=-1) 
		:index(index_) {path.push_back(rootField);}
	json_path(const std::string &rootField,const std::string &sub1Field,int index_=-1) 
		:index(index_) {path.push_back(rootField); path.push_back(sub1Field); }
	json_path(const std::string &rootField,const std::string &sub1Field,const std::string &sub2Field,int index_=-1) 
		:index(index_) {path.push_back(rootField); path.push_back(sub1Field); path.push_back(sub2Field); }
	
	// Index this json root object down to our path.  
	//  This read-only version will not create missing parts of the path,
	//  but throw exceptions.
	const json::Value &in(const json::Value &root) const {
		const json::Value *cur=&root; // use pointers because we can't re-seat references
		for (unsigned int i=0;i<path.size();i++) {
			cur=&((*cur)[path[i]]);
		}
		if (index!=-1) {
			cur=&cur->ToArray()[index];
		}
		return *cur;
	}
	
	// Index this json root object down to our path.  
	//  This read-write version will create enclosing fields.
	json::Value &in(json::Value &root) const {
		json::Value *cur=&root; // use pointers because we can't re-seat references
		for (unsigned int i=0;i<path.size();i++) {
			if (!cur->HasKey(path[i])) { // missing intermediate
				if (i==path.size()-1 && index!=-1) // array last time
					(*cur)[path[i]]=json::Array();
				else // normally intermediate object
					(*cur)[path[i]]=json::Object();
			}
			cur=&((*cur)[path[i]]);
		}
		
		if (index!=-1) {
			json::Array &a=cur->ToArray();
			if ((int)a.size()<=index) { // need to lengthen array
				while ((int)a.size()<=index) a.push_back(json::Value());
			}
			cur=&a[index];
		}
		return *cur;
	}
};

/**
 This represents a way of reading or writing stored JSON values.
*/
class json_target {
public:
	json_path path;
	json_target(const json_path &path_) :path(path_) {}
	virtual ~json_target() {}
	
	virtual void modify(json::Value &root) =0;
};

/// All sensor and command data is stored in these arrays.
tabula_control_storage tabula_sensor_storage;
tabula_control_storage tabula_command_storage;

/**
 Read this sensor from the storage array, and write it into JSON
*/
template <class jsonT,class deviceT>
class json_sensor : public json_target {
public:
	tabula_sensor<deviceT> sensor;
	json_sensor(const json_path &path_) :json_target(path_) {}
	
	virtual void modify(json::Value &root) {
		deviceT d=*(deviceT *)&tabula_sensor_storage.array[sensor.get_index()];
		path.in(root) = (jsonT)d;
	}
};

// Type conversion specialization interface
template <class jsonT,class deviceT>
deviceT json_command_conversion(const jsonT &v) { 
	return v;  // default: no conversion
}

// Motor power conversion from float [-1..+1] to signed int [-255..+255]
template <>
int16_t json_command_conversion<float,int16_t>(const float &v) { 
	int iv=(int)(255.99*v);
	     if (iv<-255) return -255;
	else if (iv>+255) return +255;
	else return (int16_t)iv;  // motor power: clamp and scale
}
// Motor power conversion from float [-1..+1] to signed char [-127..+127]
template <>
int8_t json_command_conversion<float,int8_t>(const float &v) { 
	int iv=(int)(127.99*(0.5+0.5*v));
	     if (iv<-127) return -127;
	else if (iv>+127) return 127;
	else return (int8_t)iv;  
}

// PWM conversion from float [0..1] to unsigned char [0..255]
template <>
uint8_t json_command_conversion<float,uint8_t>(const float &v) { 
	int iv=(int)(255.99*(0.5+0.5*v));
	     if (iv<0) return 0;
	else if (iv>255) return 255;
	else return (uint8_t)iv;  
}

/**
 Read this command from JSON, optionally convert it,
 and write it to the storage array.
*/
template <class jsonT,class deviceT>
class json_command : public json_target {
public:
	tabula_command<deviceT> command;
	json_command(const json_path &path_) :json_target(path_) {}
	
	virtual void modify(json::Value &root) {
		deviceT d=json_command_conversion<jsonT,deviceT>(path.in(root));
		*(deviceT *)&tabula_command_storage.array[command.get_index()]=d;
	}
};

/**
  Read commands from superstar, and send them to the robot.
*/
class robot_backend {
private:
	osl::url_parser parseURL;
	osl::http_connection superstar; // HTTP keepalive connection
	std::string robotName;
	A_packet_formatter<SerialPort> *pkt;

	// These are the central magic that lets us convert data to/from JSON:
	std::vector<json_target *> sensors;
	std::vector<json_target *> commands;
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
		for (unsigned int i=0;i< sensors.size();i++) delete  sensors[i];
		for (unsigned int i=0;i<commands.size();i++) delete commands[i];
	}
	void read_sensors(const A_packet& current_p);
	
	/** Update pilot commands from network */
	void read_network(void);
	void send_network(void);
	
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
		
		
		// This fixed table could be replaced with registration from tabula_config.h
		if (device=="serial_controller") { 
			break; // end of configuration file
		}
		else if (device=="sabertooth_v1_controller_t" 
			|| device=="sabertooth_v2_controller_t" 
			|| device=="bts_controller_t" 
			|| device=="create2_controller_t")
		{
			// Virtually all motor controllers just need motor power, left and right:
			commands.push_back(new json_command<float,int16_t>(json_path("power","L")));
			commands.push_back(new json_command<float,int16_t>(json_path("power","R")));
			
			if (device=="create2_controller_t") 
			{ // Add all the Roomba's onboard sensors
			//  These MUST match arduino/roomba.h roomba_t::sensor_t in size and order!
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("roomba","mode")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("roomba","bumper")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("battery","charge")));
				sensors.push_back(new json_sensor<int,int8_t>(json_path("battery","temperature")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("battery","voltage")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("battery","charge")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("battery","capacity")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("encoder","L")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("encoder","R")));
				for (int i=0;i<4;i++)
					sensors.push_back(new json_sensor<int,uint16_t>(json_path("roomba","floor",i)));
				for (int i=0;i<6;i++)
					sensors.push_back(new json_sensor<int,uint16_t>(json_path("roomba","light",i)));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("roomba","lightfield")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("roomba","buttons")));
			}
		}
		else if (device=="cmd") {
			continue; // skip over configuration commands
		}
		else if (device=="analog_sensor") {
			static int analogs=0;
			sensors.push_back(new json_sensor<int,uint16_t>(json_path("analog",analogs++)));
		}
		else if (device=="servo") {
			static int servos=0;
			commands.push_back(new json_command<float,uint8_t>(json_path("servo",servos++)));
		}
		else if (device=="pwm_pin") {
			static int pwms=0;
			commands.push_back(new json_command<float,uint8_t>(json_path("pwm",pwms++)));
		}
		else std::cout<<"Arduino backend: ignoring unknown device '"<<device<<"'\n";
	}
	printf("Backend configured: %d command bytes, %d sensor bytes!\n",
		(int)tabula_command_storage.count,(int)tabula_sensor_storage.count);
}

void robot_backend::setup_arduino(SerialPort &port,std::string robot_config)
{
	while (true) { // wait for Arduino to boot
		std::string start=getline_serial(port); 
		std::cout<<"Arduino startup: "<<start<<"\n";
		if (start[0]=='9') break;
	}
	port.write(&robot_config[0],robot_config.size()); // dump to Arduino
	while (true) {
		std::string status=getline_serial(port);
		if (status=="-1") break;
		std::cout<<"Arduino config: "<<status<<"\n";
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
	if (current_p.length!=tabula_sensor_storage.count) {
		printf("Device and backend mismatch!  Arduino sensors %d bytes, backend sensors %d bytes!\n",
			(int)current_p.length,(int)tabula_sensor_storage.count);
	}
	else {
		memcpy(tabula_sensor_storage.array,current_p.data,current_p.length);
	}
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
			if (debug) printf("Arduino sent packet type %x (%d bytes):\n",p.command,got_data);
			if (p.command == 0) printf("    Arduino sent echo request %d bytes, '%.*s'\n", p.length, p.length, p.data);
			else if (p.command==0xE) printf("ERROR sent from Arduino: %d bytes, '%.*s'\n", p.length, p.length, p.data);
			else if (p.command == 0xC)
			{
				if (debug) printf("    Arduino sent sensor data: %d bytes, '%.*s'\n", p.length, p.length, p.data);
				read_sensors(p);
			}
			else printf("    Arduino sent unknown packet 0x%X command %d bytes, '%.*s'\n", p.command, p.length, p.length, p.data);
		}
	}
}

/** Send data to the robot over serial connection */
void robot_backend::send_serial(void) {
	if (pkt==0) return; // simulation only
	
	pkt->write_packet(0xC,
		tabula_command_storage.count,
		tabula_command_storage.array);
}

/** Read pilot data from superstar, and store into ourselves */
void robot_backend::read_network()
{
	std::string path="/superstar/"+robotName+"/pilot?get";
	double start=time_in_seconds();
	superstar.send_get(path);
	std::string json_data=superstar.receive();
	
	try {

		json::Value v=json::Deserialize(json_data);
		
		// Pull registered commands from JSON
		for (unsigned int i=0;i< commands.size();i++) commands[i]->modify(v);

#ifndef	_WIN32
		// Script execution magic
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

/*
// Unported:
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

		if (debug)
		{
			printf("Power commands: %.2f L  %.2f R  %.2f S1  %.2f S2 %.2f S3\nLED on/demo : ", L, R, S1, S2, S3);
			printf(ledOn ? "true/" : "false/");
			printf(ledDemo ? "true\n" : "false\n");
			printf("RGB Values: %d R %d G %d B \n", led_red, led_blue, led_green);

			//printf("   Network data: %ld bytes, '%s'\n", json_data.size(), json_data.c_str());
		}
*/

	} catch (std::exception &e) {

		printf("Exception while processing network JSON: %s\n",e.what());
		printf("   Network data: %ld bytes, '%s'\n", json_data.size(),json_data.c_str());
		// stop();
	}
	double elapsed=time_in_seconds()-start;
	double per=elapsed;
	printf("\nRead Time:	%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);
}

void robot_backend::send_network(void)
{
	double start = time_in_seconds();
	std::string path = "/superstar/" + robotName + "/sensors?set="; //data from robot
	try 
	{ // send all registered sensor values
		json::Value root=json::Object();
		for (unsigned int i=0;i< sensors.size();i++) sensors[i]->modify(root);
		
		std::string str = json::Serialize(root);
		superstar.send_get(path+str); 
		std::cout<<"Sent sensor JSON: "<<str<<"\n";
	} catch (std::exception &e) {
		printf("Exception while sending network JSON: %s\n",e.what());
		// stop();
	}
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
	superstar.send_get(path+data); 
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
	std::string configMotor = "create2_controller_t X3";
	int baudrate = 57600;  // serial comms
	for (int argi = 1; argi<argc; argi++) {
		if (0 == strcmp(argv[argi], "--robot")) robotName = argv[++argi];
		else if (0 == strcmp(argv[argi], "--superstar")) superstarURL = argv[++argi];
		else if (0 == strcmp(argv[argi], "--baudrate")) baudrate = atoi(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--config")) configMotor = argv[++argi];
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
	
	
	// FIXME: should pull robot configuration from superstar robotName/+"config"
	std::string robot_config=
"analog_sensor A0\n"
"analog_sensor A5\n"

+configMotor+"\n"

#if 0 // need smarter web front end here
"servo 10\n"
"servo 9\n"
"pwm_pin 3\n"
"cmd 0 200\n"
#endif
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


