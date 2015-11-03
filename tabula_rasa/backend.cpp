/**
  The backend reads HTTP pilot commands from superstar, writes serial
  commands to the arduino, and sends sensor data back up.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-03-21 (public domain)
*/
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include <stdint.h>  // for unit8-16 and whatnot
#endif

//String helper:
#include "../include/string_util.h"

// Config file/cli read/write:
#include "../include/ini.h"
#include "../include/robot_config.h"

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
#include "../include/ini.cpp"
#include "../include/robot_config.cpp"
#include "../include/string_util.cpp"
#include "../include/serial.cpp"
#include "../include/osl/socket.cpp"
#include "../include/osl/webservice.cpp"
#include "../include/json.cpp"

#include "../include/location_binary.h" /* for computer vision marker I/O */
#include "../include/osl/vec4.h" /* for vec3, used for arithmetic */

#ifndef M_PI
# define M_PI 3.1415926535897
#endif

void moose_sleep_ms(int delay_ms)
{
#if defined(__unix__)||defined(__MACH__)
	timespec t1;
	t1.tv_sec=delay_ms/1000; // remaining seconds
	t1.tv_nsec=1000*1000*(delay_ms%1000); // nanoseconds
	nanosleep(&t1,NULL); // limit rate to 100Hz, to be kind to serial port and network
#else
	Sleep(delay_ms);
#endif
}

void clean_exit(const char *why) {
	fprintf(stderr,"Backend exiting: %s\n",why);
	exit(1);
}

int tabula_serial_begin(int baudrate) {
	std::vector<std::string> ports=Serial.port_list();

	if (ports.size()==0) { printf("ERROR!  No serial ports found!  (Is it plugged in?)\n"); return -1; }

	for(size_t ii=0;ii<ports.size();++ii)
	{
		std::cout<<ii<<":  "<<ports[ii]<<std::endl;

		if (ports[ii].find("Bluetooth")==std::string::npos)
		{
			if(0==Serial.Open(ports[ii]))
			{
				printf("\nOpened serial port %s, baud rate %d\n",ports[ii].c_str(),baudrate);
				Serial.Set_baud(baudrate);
				return 0;
			}
		}
	}

	printf("ERROR!  No serial ports were opened!  (Do you have permissions?)\n");
	return -1;
}




bool sim = false; // use --sim to enable simulation mode
bool debug = false;  // spams more output data

std::string char_to_hex(const char byte)
{
	std::ostringstream ostr;
	ostr<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(unsigned int)(unsigned char)byte;
	return ostr.str();
}

std::string uri_encode(std::string str)
{
	const static std::string dont_encode="._\":,{}[]";

	for(size_t ii=0;ii<str.size();++ii)
	{
		bool found=dont_encode.find(str[ii],0)!=std::string::npos;

		if(!found&&isdigit(str[ii])==0&&isalpha(str[ii])==0)
		{
			str.replace(ii,1,"%"+char_to_hex(str[ii]));
			ii+=2;
		}
	}

	return str;
}

/// All sensor and command data is stored in these arrays.
///   They're the raw binary data going to/from the Arduino
tabula_control_storage tabula_sensor_storage;
tabula_control_storage tabula_command_storage;



/**
  This class is used to localize the robot
*/
class robot_location {
public:
	/** Merged location */
	location_binary merged;

	/** Values from computer vision */
	location_binary vision;
	location_reader vision_reader;

	/** Update computer vision values */
	void update_vision(const char *marker_path) {
		if (vision_reader.updated(marker_path,vision)) {
			if (vision.valid) {
				merged=vision; // <- HACK!  Always trusts camera, even if it's bouncing around.
			}
		}
	}

	double coordfix(double coordinate) {
		return long(coordinate*1000.0)/1000.0; // round to mm accuracy
	}

	/* Write our location values into this robot object */
	void copy_to_json(json::Value &robot) {
		robot["location"]=json::Object();
		robot["location"]["x"]=coordfix(merged.x);
		robot["location"]["y"]=coordfix(merged.y);
		robot["location"]["z"]=coordfix(merged.z);
		robot["location"]["angle"]=(int)merged.angle; // (int) to avoid extra decimal places
		robot["location"]["count"]=(int)merged.count; // counter to verify progress
		robot["location"]["ID"]=(int)merged.marker_ID;
	}

	/* Update absolute robot position based on these incremental
	   wheel encoder distances.
	   These are normalized such that left and right are the
	   actual distances the wheels rolled, and wheelbase is the
	   effective distance between the wheels' center of traction.
	   Default units for vision_reader are in meters.
	*/
	void move_wheels(float left,float right,float wheelbase) {
	// Extract position and orientation from absolute location
		vec3 P=vec3(merged.x,merged.y,merged.z); // position of robot (center of wheels)
		double ang_rads=merged.angle*M_PI/180.0; // 2D rotation of robot

	// Reconstruct coordinate system and wheel locations
		vec3 FW=vec3(cos(ang_rads),sin(ang_rads),0.0); // forward vector
		vec3 UP=vec3(0,0,1); // up vector
		vec3 LR=FW.cross(UP); // left-to-right vector
		vec3 wheel[2];
		wheel[0]=P-0.5*wheelbase*LR;
		wheel[1]=P+0.5*wheelbase*LR;

	// Move wheels forward by specified amounts
		wheel[0]+=FW*left;
		wheel[1]+=FW*right;

	// Extract new robot position and orientation
		P=(wheel[0]+wheel[1])*0.5;
		LR=normalize(wheel[1]-wheel[0]);
		FW=UP.cross(LR);
		ang_rads=atan2(FW.y,FW.x);

	// Put back into merged absolute location
		merged.angle=180.0/M_PI*ang_rads;
		merged.x=P.x; merged.y=P.y; merged.z=P.z;
	}
};


/**
 One entry in the list of json path steps--a field or array name
*/
class json_path_fragment {
public:
	std::string field; // field name
	int index; // array index, or -1 if not an array
	json_path_fragment(const std::string &f,int index_=-1) 
		:field(f), index(index_) {}
	
	// Index this json value by our field name (r/o const)
	const json::Value *index_read(const json::Value *cur) const {
		cur=&((*cur)[field]);
		
		if (index>=0) {
			cur=&cur->ToArray()[index];
		}
		
		return cur;
	}
	// Index this json value by our field name (r/w, creates)
	json::Value *index_write(json::Value *cur,bool islast) const {
		if (!cur->HasKey(field)) { // missing intermediate
			if (index!=-1) // array next time
				(*cur)[field]=json::Array();
			else // normal intermediate object
				(*cur)[field]=json::Object();	
		}
		cur=&((*cur)[field]);
		
		if (index>=0) {
			json::Array &a=cur->ToArray();
			if ((int)a.size()<=index) { // need to lengthen array
				while ((int)a.size()<=index) 
					if (islast) a.push_back(json::Value());
					else a.push_back(json::Object());
			}
			cur=&a[index];
		}
		
		return cur;
	}
};

/**
 This is the location of one field in a JSON object,
   for example, robot.foo.bar[3] stored as {"foo","bar"}, 3
*/
class json_path {
public:
	std::vector<json_path_fragment> path;
	json_path(const char *rootField)
	{
		path.push_back(json_path_fragment(rootField));
	}
	json_path(const std::string &rootField,int index_)
	{
		path.push_back(json_path_fragment(rootField,index_));
	}
	
	json_path(const json_path &parent,const std::string &sub1Field,int index_=-1)
		:path(parent.path) 
	{ 
		path.push_back(json_path_fragment(sub1Field,index_)); 
	}
	
	json_path(const json_path &parent,const std::string &sub1Field,const std::string &sub2Field,int index_=-1)
		:path(parent.path) 
	{ 
		path.push_back(json_path_fragment(sub1Field)); 
		path.push_back(json_path_fragment(sub2Field,index_)); 
	}

	// Index this json root object down to our path.
	//  This read-only version will not create missing parts of the path,
	//  but throw exceptions.
	const json::Value &in(const json::Value &root) const {
		const json::Value *cur=&root; // use pointers because we can't re-seat references
		for (unsigned int i=0;i<path.size();i++) {
			cur=path[i].index_read(cur);
		}
		return *cur;
	}

	// Index this json root object down to our path.
	//  This read-write version will create enclosing fields.
	json::Value &in(json::Value &root) const {
			//std::cout<<"Indexing {\n";
		json::Value *cur=&root; // use pointers because we can't re-seat references
		for (unsigned int i=0;i<path.size();i++) {
			//std::cout<<"	Indexing at path "<<path[i].field<<"\n";
			cur=path[i].index_write(cur,i==path.size()-1);
		}
			//std::cout<<"} Indexing done\n";
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
};

/* Abstract superclass of robot sensors */
class json_sensor_abstract : public json_target
{
public:
	json_sensor_abstract(const json_path &path_) :json_target(path_) {}

	// Read the current value of this sensor.
	virtual void read_sensor(json::Value &sensor_root) =0;
};


/* Abstract superclass of robot commands */
class json_command_abstract : public json_target
{
public:
	json_command_abstract(const json_path &path_) :json_target(path_) {}

	// Read this commanded value from the pilot, and execute it.
	virtual void do_command(json::Value &pilot_root) =0;

	// Send our current value to this sensor structure
	virtual void do_sensor(json::Value &sensor_root) =0;
};


/**
 Read this sensor from the storage array, and write it into JSON
*/
template <class jsonT,class deviceT>
class json_sensor : public json_sensor_abstract {
public:
	tabula_sensor<deviceT> sensor;
	json_sensor(const json_path &path_) :json_sensor_abstract(path_) {}

	// Return the last-read value of our sensor
	deviceT read(void) const {
		return *(deviceT *)&tabula_sensor_storage.array[sensor.get_index()];
	}

	// Write our sensor value into this JSON object
	virtual void read_sensor(json::Value &root) {
		deviceT d=read();
		path.in(root) = (jsonT)d;
	}
};

/* Specialization to output sensor data from Neato XV-11 laser distance sensor */
#include "arduino/neato_serial.h"
template <>
class json_sensor<float,NeatoLDSbatch> : public json_sensor_abstract {
public:
	unsigned char changes; // nonce counter for updates
	int updated_count; // number of new samples received
	int last_index; // last-received batch start index
	enum {NDIR=360};
	NeatoLDSdir dirs[NDIR];
	tabula_sensor<NeatoLDSbatch> sensor;
	json_sensor(const json_path &path_) :json_sensor_abstract(path_) {
		changes=0;
		updated_count=0;
	}

	// Return the last-read value of our sensor
	NeatoLDSbatch read(void) const {
		return *(NeatoLDSbatch *)&tabula_sensor_storage.array[sensor.get_index()];
	}

	// Write our sensor value into this JSON object
	virtual void read_sensor(json::Value &root) {
		NeatoLDSbatch b=read();
		if (b.index>=0 && b.index+NeatoLDSbatch::size<=NDIR)
		{ // copy data from this latest batch into the global array
			for (int i=0;i<NeatoLDSbatch::size;i++) {
				dirs[i+b.index]=b.dir[i];
			}
			if (b.index!=last_index) {
				updated_count+=NeatoLDSbatch::size;
				if (updated_count>=360) {
					changes++;
					updated_count=0;
				}
				last_index=b.index;
			}
		}

		root["lidar"]=json::Object();
		root["lidar"]["rpm"]=b.speed64*(1.0/64.0);
		root["lidar"]["errors"]=b.errors;
		root["lidar"]["depth"]=json::Array();
		root["lidar"]["scale"]="0.001";
		root["lidar"]["change"]=changes;
		// HACK: distances are integer mm.  meters seems cleaner, but floats get printed like "0.324000", which is big.
		json::Array &a=root["lidar"]["depth"].ToArray();
		for (int i=0;i<NDIR;i++) {
			int in=i;
			if (true) in=NDIR-1-i; // flip upside down
			a.push_back(dirs[in].distance);
		}
	}
};


template <class deviceT>
class wheel_encoders : public json_sensor_abstract {
	deviceT oldL, oldR;
public:
	robot_location &location;
	json_sensor<int,deviceT> L;
	json_sensor<int,deviceT> R;
	float distance_per_count, wheelbase;
	wheel_encoders(robot_location &location_,float distance_per_count_,float wheelbase_)
		:json_sensor_abstract(json_path("encoder")), location(location_),
		 L(json_path("encoder","L")), R(json_path("encoder","R")),
		 distance_per_count(distance_per_count_), wheelbase(wheelbase_)
	{
		oldL=L.read();
		oldR=R.read();
	}

	double wraparound_fix(deviceT devDiff) {
		return (0xff&(devDiff+128))-128;
	}

	virtual void read_sensor(json::Value &root) {
		// Pass new encoder values up to robot location
		deviceT newL=L.read(), newR=R.read();
		double delL=wraparound_fix(newL-oldL), delR=wraparound_fix(newR-oldR);
		if (delL!=0 || delR!=0) {
			location.move_wheels(
				delL*distance_per_count,
				delR*distance_per_count,
				wheelbase);
		}
		oldL=newL; oldR=newR;

		// Pass value out to JSON
		L.read_sensor(root);
		R.read_sensor(root);
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

// Servo or PWM conversion from int to unsigned char [0..255]
template <>
uint8_t json_command_conversion<int,uint8_t>(const int &v) {
	int iv=(int)(v);
	     if (iv<0) return 0;
	else if (iv>255) return 255;
	else return (uint8_t)iv;
}

// Wraparound conversion to unsigned char [0..255]
template <>
int8_t json_command_conversion<int,int8_t>(const int &v) {
	return (int8_t)v;
}


/**
 Read this command from JSON, optionally convert it,
 and write it to the storage array.
*/
template <class jsonT,class deviceT>
class json_command : public json_command_abstract {
public:
	jsonT scaleFactor; // from JSON to device value
	tabula_command<deviceT> command;
	json_command(const json_path &path_,jsonT scaleFactor_=1.0)
		:json_command_abstract(path_), scaleFactor(scaleFactor_) { }

	virtual void do_command(json::Value &pilot_root)
	{
		deviceT d=0;
		try {
			json::Value &pilot_raw=path.in(pilot_root); // read commanded value from pilot
			if (pilot_raw.IsNumeric()) {
				jsonT j=pilot_raw; // convert to number
				j*=scaleFactor;
				d=json_command_conversion<jsonT,deviceT>(j);
			}
		} catch (std::runtime_error &e) {
			// not available from pilot or malformed--ignore value.
			std::cout<<"Ignoring pilot json error "<<e.what()<<"\n";
		}
		
		// Write value into command array
		*(deviceT *)&tabula_command_storage.array[command.get_index()]=d;
	}

	// Send our current value to this sensor structure
	virtual void do_sensor(json::Value &sensor_root)
	{
		// Read value from command array
		deviceT d=*(deviceT *)&tabula_command_storage.array[command.get_index()];
		jsonT j=(jsonT)d;
		path.in(sensor_root)=j;
	}
};

/**
  Read commands from superstar, and send them to the robot.
*/
class robot_backend {
private:
	osl::url_parser parseURL;
	osl::http_connection superstar; // HTTP keepalive connection
	std::string superstar_send_get(const std::string &path); // HTTP request

	std::string robotName;
	A_packet_formatter<SerialPort> *pkt;

	// All supported tabula devices, with their argument list.
	json::Array all_dev_types;

	// All the configured device strings, as parsed by setup_devices
	std::vector<std::string> robot_config_devices;

	// These are the central magic that lets us convert data to/from JSON:
	//   an array of configure types.
	std::vector<json_sensor_abstract *> sensors;
	std::vector<json_command_abstract *> commands;
public:
	robot_location location;
	double LRtrim;
	bool debug;
	int config_counter;
	bool _timeout;
	int _serialOpen;
	bool gotData;

	robot_backend(std::string superstarURL, std::string robotName_)
		:parseURL(superstarURL), superstar(parseURL.host,0,parseURL.port),
		robotName(robotName_), pkt(0), LRtrim(1.0),config_counter(-1234)
	{
		gotData=false;
		//stop();
	}
	~robot_backend() {
		delete pkt;
		for (unsigned int i=0;i< sensors.size();i++) delete  sensors[i];
		for (unsigned int i=0;i<commands.size();i++) delete commands[i];
	}
	void read_sensors(const A_packet& current_p);

	/** Update pilot commands from network */
	void do_network(void);
	void read_network(const std::string &read_json);
	std::string send_network(void);

	void tabula_setup(std::string config);
	void read_config(std::string config,const json::Value& configs,const int counter);
	void send_config(std::string config);

	void send_options(void);

	// Configure these devices
	void setup_devices(std::string robot_config);

	// Pull arduino's firmware options
	void get_arduino_options(SerialPort &port);

	/** Talk to this real Arudino device over this serial port.
	    Run this robot configuration. */
	void setup_arduino(SerialPort &port,std::string robot_config);

	/** Send pilot commands to robot. */
	void send_serial();
	/** Read anything the robot wants to send to us. */
	void read_serial();
	
	/** Try to reconnect if arduino is unplugged*/
	void reconnect (std::string config, SerialPort &port);
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
	all_dev_types.Clear();
	robot_config_devices=std::vector<std::string>();

	// Clear existing lists of sensors and actuators
	for (unsigned int i=0;i<commands.size();i++) delete commands[i];
	commands.clear();
	tabula_command_storage.clear();

	for (unsigned int i=0;i<sensors.size();i++) delete sensors[i];
	sensors.clear();
	tabula_sensor_storage.clear();

	// Counters for various devices:
	int analogs=0;
	int servos=0;
	int pwms=0;
	int blinks=0;
	int neopixels=0;

	// Parse lines of the configuration outselves, to
	//   find the command and sensor fields and match them to JSON
	std::istringstream robot_config_stream( robot_config );
	while (robot_config_stream) {
		std::string device="", args="";
		std::ws(robot_config_stream); std::getline(robot_config_stream,device,'(');
		std::ws(robot_config_stream); std::getline(robot_config_stream,args,')');
		char last=0; robot_config_stream>>last;
		printf("Config: device '%s', args '%s', ends with %c\n",
			device.c_str(), args.c_str(), last);
		if (last!=';') {
			std::cout<<"Configuring backend: missing terminating semicolon on "<<device<<"!\n";
			robot_config_stream.unget(); // put char back
		}
		robot_config_devices.push_back(device+"("+args+");");

		// This fixed table could be replaced with registration from tabula_config.h
		if (device=="serial_controller") {
			break; // end of configuration file
		}
		else if (device=="sabertooth1"
			|| device=="sabertooth2"
			|| device=="bts"
			|| device=="create2")
		{
			// Virtually all motor controllers just need motor power, left and right:
			commands.push_back(new json_command<float,int16_t>(json_path("power","L")));
			commands.push_back(new json_command<float,int16_t>(json_path("power","R"),LRtrim));

			if (device=="create2")
			{ // Add all the Roomba's onboard sensors
			//  These MUST match arduino/roomba.h roomba_t::sensor_t in size and order!
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("mode")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("bumper")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("battery","state")));
				sensors.push_back(new json_sensor<int,int8_t>(json_path("battery","temperature")));
				sensors.push_back(new json_sensor<int,uint16_t>(json_path("battery","charge")));

				sensors.push_back(new wheel_encoders<uint16_t>(location,
					0.0004444, // wheel travel distance (m) per encoder count
					0.235 // roomba's wheelbase (lower effective wheelbase on carpet)
					));

				for (int i=0;i<4;i++)
					sensors.push_back(new json_sensor<int,uint8_t>(json_path("floor",i)));
				for (int i=0;i<6;i++)
					sensors.push_back(new json_sensor<int,uint8_t>(json_path("light",i)));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("light_field")));
				sensors.push_back(new json_sensor<int,uint8_t>(json_path("buttons")));
			}
		}
		else if (device=="neato") {
			sensors.push_back(new json_sensor<float,NeatoLDSbatch>(json_path("lidar")));
		}
		else if (device=="cmd") {
			continue; // skip over configuration commands
		}
		else if (device=="latency") {
			sensors.push_back(new json_sensor<int,uint8_t>(json_path("latency")));
		}
		else if (device=="analog") {
			sensors.push_back(new json_sensor<int,uint16_t>(json_path("analog",analogs++)));
		}
		else if (device=="servo") {
			commands.push_back(new json_command<int,uint8_t>(json_path("power","servo",servos++)));
		}
		else if (device=="pwm") {
			commands.push_back(new json_command<int,uint8_t>(json_path("power","pwm",pwms++)));
		}
		else if (device=="neopixel") {
			json_path np("power","neopixel",neopixels++);
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"color","r")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"color","g")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"color","b")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"accent","r")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"accent","g")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"accent","b")));
			commands.push_back(new json_command<int, int8_t>(json_path(np,"start")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"repeat")));
			commands.push_back(new json_command<int,uint8_t>(json_path(np,"state")));
		}
		else if (device=="blink") {
			commands.push_back(new json_command<int,uint8_t>(json_path("power","blink",blinks++)));
		}
		else if (device=="heartbeat") {
			sensors.push_back(new json_sensor<int,uint8_t>(json_path("heartbeats")));
		}
		else if (device=="bms") {
			sensors.push_back(new json_sensor<int,uint8_t>(json_path("battery","charge")));
			sensors.push_back(new json_sensor<int,uint8_t>(json_path("battery","state")));
		}
		else std::cout<<"Arduino backend: ignoring unknown device '"<<device<<"'\n";
	}

	// Clear out command and sensor bytes (otherwise they're full of junk)
	memset(tabula_command_storage.array,0,tabula_command_storage.count);
	memset(tabula_sensor_storage.array,0,tabula_sensor_storage.count);

	printf("Backend configured: %d command bytes, %d sensor bytes!\n",
		(int)tabula_command_storage.count,(int)tabula_sensor_storage.count);
}

void robot_backend::get_arduino_options(SerialPort &port) 
{
	std::cout.flush();

	while (true) { // wait for Arduino to boot
		(std::cout<<"Arduino startup: ").flush();
		std::string start=getline_serial(port);
		std::cout<<start<<"\n";
		if (start.find("tabula device names")!=std::string::npos) break;
	}

	// Pull Arduino's current options list
	port.write("list\n",5);
	std::string count_str=getline_serial(port);
	int count=atoi(count_str.c_str());
	if (count_str.size()<1 || count<1) {
		std::cerr<<"Expected device count, got '"<<count<<"'.\n";
		std::cerr<<"Invalid Arduino device list--do you need to flash the latest tabula_rasa firmware?\n";
	}
	for (int dev=0;dev<count;dev++) {
		std::string option=getline_serial(port);
		std::cout<<"  Arduino device supported: '"<<option<<"'\n";
		if (option!="serial_controller ")
			all_dev_types.push_back(option);
	}
}

void robot_backend::setup_arduino(SerialPort &port,std::string robot_config)
{
	// Dump configuration to Arduino
	for (unsigned int i=0;i<robot_config_devices.size();i++) {
		// Send device to Arduino
		std::string dev=robot_config_devices[i];
		std::cout<<"  To Arduino: "<<dev<<"\n";
		port.write(&dev[0],dev.size());

		// Wait for device creation confirmation from Arduino
		std::string status="";
		do {
			status=getline_serial(port);
			std::cout<<"  From Arduino: "<<status<<"\n";
			if (status=="-1") break;
		} while (status[0]!='1');
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
		printf("Do you need to flash the Arduino again?");
		clean_exit("Arduino does not match backend");
	}
	else {
		memcpy(tabula_sensor_storage.array,current_p.data,current_p.length);
	}
}

/** Read anything the robot wants to send to us. */
void robot_backend::read_serial(void) {
	if (pkt==0) return; // simulation only
	
	int got_data=0;
	
	while (Serial.available()) { // read any robot response
		if (debug) printf("v");

		A_packet p;
		while (-1==pkt->read_packet(p)) { gotData = true;}
		if (p.valid) {
			//if (debug) 
			std::cout<<"P";
			if (debug) printf("Arduino sent packet type %x (%d bytes):\n",p.command,got_data);
			if (p.command == 0) printf("    Arduino sent echo request %d bytes, '%.*s'\n", p.length, p.length, p.data);
			else if (p.command==0xE) {
				printf("ERROR sent from Arduino: %d bytes, '%.*s'\n", p.length, p.length, p.data);
				clean_exit("Arduino sent error report");
			}
			else if (p.command==0xD) {
				printf("DEBUG sent from Arduino: %d bytes, '%.*s'\n", p.length, p.length, p.data);
			}
			else if (p.command == 0xC)
			{
				if (debug) printf("    Arduino sent sensor data: %d bytes, '%.*s'\n", p.length, p.length, p.data);
				read_sensors(p);
			}
			else printf("    Arduino sent unknown packet 0x%X command %d bytes, '%.*s'\n", p.command, p.length, p.length, p.data);
		}
	}
	if(gotData)
	{
		_timeout=0;
		_serialOpen = 1;
		std::cout<<"Timeout: "<<_timeout<<std::endl;
	}
	else 
	{
		_timeout++;
		std::cout<<"Timeout: "<<_timeout<<std::endl;
	}
	
}

/** Send data to the robot over serial connection */
void robot_backend::send_serial(void) {
	if (pkt==0) return; // simulation only

	pkt->write_packet(0xC,
		tabula_command_storage.count,
		tabula_command_storage.array);
}

/** Send this HTTP get request for this path, and return the resulting string. */
std::string robot_backend::superstar_send_get(const std::string &path)
{
	for (int run=0;run<5;run++) {
		try {
			superstar.send_get(path);
			return superstar.receive();
		} catch (skt_error &e) {
			std::cout<<"NETWORK ERROR! "<<e.what()<<std::endl;
			std::cout<<"Retrying connection to superstar "<<parseURL.host<<":"<<parseURL.port<<std::endl;
			// Reopen HTTP connection
			superstar.close(); // close old connection
			if (run>0) sleep(1);
			// make new connection (or die trying)
			superstar.connect(parseURL.port,60+30*run);
			std::cout<<"Reconnected to superstar!\n";
		}
	}
	clean_exit("NETWORK ERROR talking to superstar.  Do you have wireless?");
	return "network error";
}

/**
 Do our network roundtrip to superstar.
*/
void robot_backend::do_network()
{
	double start=time_in_seconds();

	std::cout << "\033[2J\033[1;1H"; // clear screen	
	std::cout<<"Robot name: "<<robotName<<"\n";

	std::string send_json=send_network();
	std::cout<<"Outgoing sensors: "<<send_json<<"\n";

	std::string send_path=robotName+"/sensors";
	std::string read_path=robotName+"/pilot,"+robotName+"/config";
	std::string request=send_path+"?set="+send_json+"&get="+read_path;
	std::string read_json=superstar_send_get("/superstar/"+request);

	std::cout<<"Incoming pilot commands: "<<read_json<<"\n";
	read_network(read_json);

	double elapsed=time_in_seconds()-start;
	double per=elapsed;
	std::cout<<"Superstar:	"<<std::setprecision(1)<<per*1.0e3<<" ms/request, "<<1.0/per<<" req/sec\n\n";
	std::cout.flush();
}

/** Read this pilot data from superstar, and store into ourselves */
void robot_backend::read_network(const std::string &read_json)
{
	try {
		json::Array return_json=json::Deserialize(read_json);

		if(return_json.size()!=2)
			throw std::runtime_error("Invalid json received (expected 2 arguments) - "+read_json);

		json::Value pilot=return_json[0];
		json::Object config=return_json[1];

		if(config["configs"].GetType()==json::ArrayVal&&config["configs"].GetType()==json::ArrayVal&&config["counter"].IsNumeric())
			read_config("",config["configs"].ToArray(),config["counter"].ToInt());

		// Pull registered commands from JSON
		for (unsigned int i=0;i< commands.size();i++) commands[i]->do_command(pilot);

#ifndef	_WIN32
		//  On UNIX systems, run shell scripts from script/ directory
		static std::string last_cmd_arg="";
		static bool startup=true;
		std::string cmd_arg="";
		if(pilot["cmd"].GetType()==json::ObjectVal)
		{
			std::string run=pilot["cmd"]["run"];
			std::string arg=pilot["cmd"]["arg"];

			if (run.size()>0 && run.find_first_of("./\\\"")==std::string::npos) { // looks clean
				cmd_arg=run+arg;
				if (last_cmd_arg!=cmd_arg && !startup) { // new script command: run it

					// std::string path="./"+run;
					printf("RUNNING SCRIPT: '%s' with arg '%s'\n",
						run.c_str(),arg.c_str());

					if (fork()==0) {
						if (chdir("script")!=0) {
							printf("SCRIPT chdir FAILED\n");
						}
						else {
							execl("logger","logger",run.c_str(),arg.c_str(),(char *)NULL);
							perror("SCRIPT EXECUTE FAILED\n");
						}
						exit(0);
					}
				}
			}
		}
		startup=false;
		last_cmd_arg=cmd_arg;

#endif

		if (sim) {
			double distance_per_power=0.02; // meters per timestep
			double wheelbase=0.3; // meters

			if(pilot["power"].GetType()==json::ObjectVal&&pilot["power"]["L"].IsNumeric()&&pilot["power"]["R"].IsNumeric())
			{
				double delL=pilot["power"]["L"];
				double delR=pilot["power"]["R"];

				location.move_wheels(
					delL*distance_per_power,
					delR*distance_per_power,
					wheelbase);
			}
		}

	} catch (std::exception &e) {

		printf("Exception while processing network JSON: %s\n",e.what());
		printf("   Network data: %ld bytes, '%s'\n", (long)read_json.size(),read_json.c_str());
		// stop();
	}
}

/** Get the sensor reports to send across the network */
std::string robot_backend::send_network(void)
{
	std::string send_json="";
	try
	{ // send all registered sensor values
		json::Value sensorsJ=json::Object();
		location.copy_to_json(sensorsJ);
		for (unsigned int i=0;i< sensors.size();i++) sensors[i]->read_sensor(sensorsJ);
		for (unsigned int i=0;i< commands.size();i++) commands[i]->do_sensor(sensorsJ);

		send_json = json::Serialize(sensorsJ);
		send_json=uri_encode(send_json);
	} catch (std::exception &e) {
		printf("Exception while preparing network JSON to send: %s\n",e.what());
		// stop();
	}

	return send_json;
}

void robot_backend::tabula_setup(std::string config)
{
	config+="\nheartbeat();\n";
	config+="\nserial_controller();\n";
	setup_devices(config);

	if(!sim)
	{
		std::cout<<"Uploading new config to arduino!"<<std::endl;
		Serial.Close();
		// sleep(1);
		tabula_serial_begin(Serial.Get_baud());
		get_arduino_options(Serial);
		setup_arduino(Serial,config);
	} else { // sim mode: fake a bunch of options
		all_dev_types.push_back("analog P");
		all_dev_types.push_back("servo P");
		all_dev_types.push_back("pwm P");
		all_dev_types.push_back("create2 S");
		all_dev_types.push_back("neato SP");
		all_dev_types.push_back("neopixel PC");
		all_dev_types.push_back("bts PPPP");
		all_dev_types.push_back("bms");
	}
	send_options();
}

void robot_backend::read_config(std::string config,const json::Value& configs,const int counter)
{
	std::string path = "/superstar/" + robotName + "/config?get";
	try
	{
		for(size_t ii=0;ii<configs.ToArray().size();++ii)
			config+=configs.ToArray()[ii].ToString()+"\n";

		if(config_counter!=counter)
		{
			config_counter=counter;
			tabula_setup(config);
		}

	} catch (std::exception &e) {
		printf("Exception while sending netwdork JSON: %s\n",e.what());
		// stop();
	}

	std::cout<<"config:  \n"<<config<<std::endl;
}

void robot_backend::send_config(std::string config)
{
	double start = time_in_seconds();
	std::string path = "/superstar/" + robotName + "/config?set=";
	try
	{ // send all registered tabula devices
		json::Object json;
		json["counter"]=config_counter;
		json["configs"]=json::Array();

		std::string temp="";

		if(config.size()>0||config[config.size()-1]!='\n')
			config+="\n";

		for(size_t ii=0;ii<config.size();++ii)
		{
			if(config[ii]=='\n')
			{
				while(temp.size()>0&&isspace(temp[0])!=0)
					temp=temp.substr(1,temp.size());

				while(temp.size()>0&&isspace(temp[temp.size()-1])!=0)
					temp=temp.substr(0,temp.size()-1);

				if(temp.size()>0)
					json["configs"].ToArray().push_back(temp);

				temp="";
				continue;
			}

			temp+=config[ii];
		}

		std::string str = json::Serialize(json);
		str=uri_encode(str);
		std::string response = superstar_send_get(path+str);
		std::cout<<"Sent options JSON: "<<str<<"\n";
	} catch (std::exception &e) {
		printf("Exception while sending network JSON: %s\n",e.what());
		// stop();
	}

	double elapsed = time_in_seconds() - start;
	double per = elapsed;
	printf("Send Time:	%.1f ms/request, %.1f req/sec\n", per*1.0e3, 1.0 / per);
}

void robot_backend::send_options(void)
{
	double start = time_in_seconds();
	std::string path = "/superstar/" + robotName + "/options?set=";
	try
	{ // send all registered tabula devices
		std::string str = json::Serialize(all_dev_types);
		str=uri_encode(str);
		std::string response = superstar_send_get(path+str);
		std::cout<<"Sent options JSON: "<<str<<"\n";
	} catch (std::exception &e) {
		printf("Exception while sending network JSON: %s\n",e.what());
		// stop();
	}

	double elapsed = time_in_seconds() - start;
	double per = elapsed;
	printf("Send Time:	%.1f ms/request, %.1f req/sec\n", per*1.0e3, 1.0 / per);
}

void robot_backend::reconnect (std::string config, SerialPort &Serial)
{
	
	while(tabula_serial_begin(Serial.Get_baud())!=0)
	{
		setup_devices(config);
		std::cout<<"Attempting to Reconnect"<<std::endl;
		Serial.Close();
		// sleep(1);
		tabula_serial_begin(Serial.Get_baud());
	}
		get_arduino_options(Serial);
		setup_arduino(Serial,config);
}

robot_backend *backend=NULL; // the singleton robot

int main(int argc, char *argv[])
{
	try
	{
		// MacOS runs double-clicked programs from homedir,  
		//   so cd to directory where this program lives.
		std::string exe_name=argv[0];
		std::cout<<"Executable name: "<<exe_name<<"\n";
		std::string dir_name=exe_name;
		while (dir_name.length()>0) {
			char c=*dir_name.rbegin(); // last letter
			if (c=='/' || c=='\\') break;
			else dir_name=dir_name.substr(0,dir_name.length()-1);
		}
		std::cout<<"Directory name: "<<dir_name<<"\n";
		if (chdir(dir_name.c_str())) { /* ignore chdir errors */ }
		
		
		robot_config_t config;

		config.from_file("config.txt");
		config.from_cli(argc,argv);

		sim=to_bool(config.get("sim"));
		debug=to_bool(config.get("debug"));

		std::cout<<"Connecting to superstar at "<<config.get("superstar")<<std::endl;

		if(to_int(config.get("baudrate"))>0)
			Serial.Set_baud(to_int(config.get("baudrate")));

		backend=new robot_backend(config.get("superstar"),config.get("robot"));
		backend->LRtrim=to_double(config.get("trim"));
		backend->debug=debug;
		
		// Manually configuring Arduino (not via web) is a bad idea...
		backend->tabula_setup(config.get("sensors")+"\n"+config.get("motors"));

		// talk to robot via backend
		while(true)
		{
			backend->do_network();
			backend->send_serial();
			moose_sleep_ms(to_int(config.get("delay_ms")));
			backend->read_serial();
			
			if((backend->_timeout)!=0)
			{	
				std::cout<<"Disconnected"<<std::endl;
				backend->reconnect(config.get("sensors")+"\n"+config.get("motors"),Serial);
			}
		}
		
		if(config.get("marker")!="")
		{
			backend->location.update_vision(config.get("marker").c_str());
		}
	}

	catch(std::exception& error)
	{
		std::cout<<"ERROR! "<<error.what()<<std::endl;
		return 1;
	}

	return 0;
}
