/**
  The backend reads HTTP commands from superstar, and writes serial
  commands to the arduino.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-10-11 (public domain)
*/
#include <stdexcept>
#include <string>

// Arduino comms:
#include "../firmware/robot.h" // robot structs
#include "../firmware/serial_packet.h" // serial comms format
#include "../../include/serial.h" // serial port access

// Network comms:
#include "../../include/osl/time_function.h" // timing
#include "../../include/osl/webservice.h" // web client
#include "../../include/json.h" // JSON parsing ("super easy JSON" library)

//simulator
// #include <thread>
#include "../../include/spritelib/spritelib.h" // 2d graphics
#include "../../include/osl/vec2.h" // 2d vector
// #include "../../layla/firmware/simable_serial_packet.h" // not needed

// Script execution
#ifndef	_WIN32
#include <unistd.h> /* for fork(), execl() */
#endif


/* Do linking right here */
#include "../../include/serial.cpp"
#include "../../include/osl/socket.cpp"
#include "../../include/osl/webservice.cpp"
#include "../../include/json.cpp"
#include "../../include/spritelib/spritelib.cpp"


double limit_power(double raw) {
	double m=0.3;
	if (raw>m) return m;
	if (raw<-m) return -m;
	else return raw;
}

struct sensors {
	//rg45 8 pin setup
	//vcc
	//data back
	int16_t uSound1; // not array so json looks good
	int16_t uSound2;
	int16_t uSound3;
	int16_t uSound4;
	int16_t uSound5;
	//ground
	sensors() { uSound1=uSound2=uSound3=uSound4=uSound5=0; }
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
public:
	double L,R,S1,S2,S3; // current power values from pilot
	double LRtrim; // motor speed bias scaling for left wheel
	sensors current_sensors; //all the sensors data
	bool ledOn, ledDemo;
	bool debug; // are we in debug mode
	int16_t led_red,led_green,led_blue; // LED RGB values send by pilot

	void stop(void) { L=R=0.0; }

	robot_backend(std::string superstarURL, std::string robotName_)
		:parseURL(superstarURL), superstar(parseURL.host,0,parseURL.port),
		robotName(robotName_), pkt(0), LRtrim(1.0)
	{
		stop();
	}
	~robot_backend() {
		delete pkt;
	}
	void read_sensors(const A_packet& current_p);
	/** Update pilot commands from network */
	void read_network(void);
	void send_network(void);
	/** Talk to this real Arudino device over this serial port.
		We will delete the packet formatter when destructed.
		The serial commands are no-ops until this is run.
	*/
	void add_serial(A_packet_formatter<SerialPort> *pkt_) { pkt=pkt_; }
	/** Send pilot commands to robot. */
	void send_serial();
	/** Read anything the robot wants to send to us. */
	void read_serial(void);
};

void robot_backend::read_sensors(const A_packet& current_p)
{
	robot_sensors_fast sensor_data; // temp
	if (current_p.get(sensor_data))// valid sesnor packet and copy to our temp
	{
		current_sensors.uSound1 = sensor_data.uSound1;
		current_sensors.uSound2 = sensor_data.uSound2;
		current_sensors.uSound3 = sensor_data.uSound3;
		current_sensors.uSound4 = sensor_data.uSound4;
		current_sensors.uSound5 = sensor_data.uSound5;
		if (debug)
		{
			printf("Ultrasound values 1,2,3,4,5 \n");
			printf("%d , %d , %d ,%d ,%d \n", current_sensors.uSound1, current_sensors.uSound2, current_sensors.uSound3, current_sensors.uSound4, current_sensors.uSound5);
		}
	}
	else
		printf("error with the serial sensor data we got, could not convert");
}
/** Read anything the robot wants to send to us. */
void robot_backend::read_serial(void) {
	if (pkt==0) return; // simulation only
	while (Serial.available()) { // read any robot response
		if (debug) printf("A");
		int16_t got_data=0;
		A_packet p;
		while (-1==pkt->read_packet(p))
		{
			//got_data++;   //packet p.length will give us this
			if (debug) printf("S");
		}
		if (p.valid) {
			if (debug) printf("P");
			printf("Arduino sent packet type %x (%d bytes):\n",p.command,got_data); // last printf give all of this (no longer need)
			if (p.command == 0) printf("    Arduino sent echo request %d bytes, '%s'\n", p.length, p.data);
			else if (p.command==0xE) printf("    Arduino sent error packet: %d bytes, '%s'\n", p.length,p.data);
			else if (p.command == 0x5)
			{
				//printf("    Arduino sent sensor data: %d bytes, '%s'\n", p.length, p.data);
				read_sensors(p);
			}
			else if (p.command == 0x4) printf("    Arduino sent slow sensor data: %d bytes, '%s'\n", p.length, p.data);
			else printf("    Arduino sent unknown packet %d command %d bytes, '%s'\n",p.command, p.length, p.data);
		}
	}
}

/** Send data to the robot over serial connection */
void robot_backend::send_serial(void) {
	if (pkt==0) return; // simulation only

	robot_power power;
	static robot_led led;  // does not need to defualt off unlike power
	//robot_led new_led; // always sending dont need this right now

	power.left=(int16_t)(64+63*L);
	power.right=(int16_t)(64+63*R);
	power.front = (int16_t)(127*(S1+.5));
	power.mine = (int16_t)(127* (S2+.5));
	power.dump = (int16_t)(127* (S3+.5));

	led.red = (int16_t)(led_red);
	led.green = (int16_t)(led_green);
	led.blue = (int16_t)(led_blue);
	led.ledon = (bool)(ledOn);
	led.demo = (bool)(ledDemo);

	if (debug) printf("Sending serial data\n");
	//if (memcmp(&led, &new_led, sizeof(led)) == 0)  //did it change
	//{
		//led = new_led;
		pkt->write_packet(0xC, sizeof(led), &led); //led
		//printf("sent rgb stuff");
	//}
	pkt->write_packet(0x7,sizeof(power),&power); //motor
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
	double elapsed=time_in_seconds()-start;
	double per=elapsed;
	printf("\nRead Time:	%.1f ms/request, %.1f req/sec\n",per*1.0e3, 1.0/per);
}

void robot_backend::send_network(void)
{
	double start = time_in_seconds();
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
	double elapsed = time_in_seconds() - start;
	double per = elapsed;
	printf("Send Time:	%.1f ms/request, %.1f req/sec\n", per*1.0e3, 1.0 / per);
}

robot_backend *backend=NULL; // the singleton robot

int main(int argc, char *argv[])
{
	double LRtrim=1.0;
	bool sim = false; // use --sim to enable simulation mode'
	bool debug = false;  // spams more output data
	std::string superstarURL = "http://sandy.cs.uaf.edu/";
	std::string robotName = "layla/uaf";
	int baudrate = 9600;  // serial comms
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
	backend->debug = debug; // more output, more messs, but more fixing
	if (!sim) {
		Serial.begin(baudrate);
		backend->add_serial(new A_packet_formatter<SerialPort>(Serial));
	}

	if (sim == true)
	{ // run GUI in a separate thread (disabled for non-C++11 compile)
	//	std::thread sim(spritelib_run, "SpriteLib Demo", 800, 600);     // spritelib sim
	//	sim.detach();
	}

	while (1) { // talk to robot via backend
		backend->read_network();
		backend->send_serial();
		backend->read_serial();
		backend->send_network();
#ifdef __unix__
		usleep(10*1000); // limit rate, to be kind to serial port and network
#endif
	}

	return 0;
}


/**
  Simulates the robot onscreen.
*/
class robot_simulator {
public:
	float wheelbase; // feet between center of wheels (effective distance)
	vec2 locL,locR; // location of robot's left and right wheels, in *feet*
	vec2 caster; // last-known location of caster wheel
	vec2 cast_dir; // travel direction of caster wheel

	robot_simulator() :wheelbase(1.6),
		locL(0.0,0.0), locR(wheelbase,0.0), caster(0,-100), cast_dir(0.0,1.0) {}

	void draw(spritelib &lib,const spritelib_tex &tex,const vec2 &loc,float size,float angle_rads) {
		float angle_deg=angle_rads*(180.0/3.141592);
		vec2 l=loc*30.0;
		lib.draw(tex,400+l.x,300-l.y,size,size,angle_deg);
	}

	void simulate(spritelib &lib) {
		// Drive wheels forward
		vec2 across=normalize(locR-locL); // robot right hand direction
		vec2 forward(-across.y,across.x); // counterclockwise perpendicular vector
		double speed=6.0; // feet/sec at power==1.0
		locL+=lib.dt*speed*backend->L*forward;
		locR+=lib.dt*speed*backend->R*forward;

		// Force distance between wheels to be constant
		vec2 cen=0.5*(locR+locL); // center
		vec2 dir=locR-locL; // faces right
		dir=normalize(dir)*wheelbase;
		locL=cen-0.5*dir;
		locR=cen+0.5*dir;

		// Update coordinate system
		across=normalize(locR-locL);
		forward=vec2(-across.y,across.x);

		// Calculate location of caster wheel
		double caster_dist=1.5; // feet
		vec2 new_caster=cen-forward*caster_dist;
		vec2 rel_caster=new_caster-caster;
		if (length(rel_caster)>0.05) {
			caster=new_caster;
			double caster_speed=4.0;
			cast_dir=normalize(cast_dir+caster_speed*lib.dt*normalize(rel_caster));
		}

		// Draw robot onscreen
		static spritelib_tex wheel=lib.read_tex("tire.png");
		float ang=atan2(dir.y,dir.x);
		draw(lib,wheel,locL,40,ang);
		draw(lib,wheel,locR,40,ang);
		draw(lib,wheel,new_caster-0.5*cast_dir,30,atan2(-cast_dir.x,cast_dir.y));
	}
};

robot_simulator *sim=0;
void spritelib_draw_screen(spritelib &lib)
{
	if (sim==0) sim=new robot_simulator();
	sim->simulate(lib);
}

