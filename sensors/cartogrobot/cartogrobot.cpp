/**
  Simple mapping system that reads HTTP sensor data from superstar, adds lidar data
  into a probabilistic occupancy map structure, and draws the map to a window.

  Aven Bross, dabross@alaska.edu, 2016-04-10
  Max Hesser-Knoll, mjhesserknoll@alaska.edu, 2016-02-15
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
#include "../../include/string_util.h"

// Config file/cli read/write:
#include "../../include/ini.h"
#include "../../include/robot_config.h"

// Network comms:
#include "../../include/osl/time_function.h" // timing
#include "../../include/osl/webservice.h" // web client
#include "../../include/json.h" // JSON parsing ("super easy JSON" library)

// Probabilistic occupancy map
#include "occupancy_grid/occupancy_grid.h"

// Script execution
#ifndef	_WIN32
#include <unistd.h> /* for fork(), execl() */
#endif

/* Do linking right here */
#include "../../include/ini.cpp"
#include "../../include/robot_config.cpp"
#include "../../include/string_util.cpp"
#include "../../include/osl/socket.cpp"
#include "../../include/osl/webservice.cpp"
#include "../../include/json.cpp"

#include "occupancy_grid/occupancy_grid.cpp"
#include "occupancy_grid/location.cpp"
#include "occupancy_grid/angle.cpp"

//OpenGL and GLUT includes
#include <cstdlib>
using std::exit;
#ifndef __APPLE__
# include <GL/glut.h>
#else
# include <GLUT/glut.h>
#endif

//Extra includes
#include "graphics/bitmapprinter.h"

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

occupancy_grid map;
map_location baseLocation;
map_location robotLocation;

/**
  Read commands from superstar, and send them to the robot.
*/
class slam_backend {
private:
	osl::url_parser parseURL;
	osl::http_connection superstar; // HTTP keepalive connection
	std::string superstar_send_get(const std::string &path); // HTTP request

	std::string robotName;

public:
	slam_backend(std::string superstarURL, std::string robotName_)
		:parseURL(superstarURL), superstar(parseURL.host,0,parseURL.port),
		robotName(robotName_)
	{
		//stop();
	}
	
	void read_config(std::string config,const json::Value& configs,const int counter);
	
	int config_counter;

	/** Update pilot commands from network */
	void do_network(void);
	void read_network(const std::string &read_json);
};

/** Send this HTTP get request for this path, and return the resulting string. */
std::string slam_backend::superstar_send_get(const std::string &path)
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

void slam_backend::read_config(std::string config,const json::Value& configs,const int counter)
{
	std::string path = "/superstar/" + robotName + "/config?get";
	try
	{
		for(size_t ii=0;ii<configs.ToArray().size();++ii)
			config+=configs.ToArray()[ii].ToString()+"\n";

		if(config_counter!=counter)
		{
			config_counter=counter;
			//tabula_setup(config);
		}

	} catch (std::exception &e) {
		printf("Exception while sending netwdork JSON: %s\n",e.what());
		// stop();
	}

	std::cout<<"config:  \n"<<config<<std::endl;
}

/**
 Do our network roundtrip to superstar.
*/
void slam_backend::do_network()
{
	//std::cout << "\033[2J\033[1;1H"; // clear screen	
	//std::cout<<"Robot name: "<<robotName<<"\n";

	std::string read_path=robotName+"/sensors";
	std::string read_json=superstar_send_get("/superstar/"+read_path+"?get");	//+request);

	//std::cout<<"Incoming sensor values: "<<read_json<<"\n";
	read_network(read_json);
	
	//std::cout<<"Superstar:	"<<std::setprecision(1)<<per*1.0e3<<" ms/request, "<<1.0/per<<" req/sec\n\n";
	//std::cout << "x: " << robotLocation.get_x() << ", y: " << robotLocation.get_y() << "\n";
	//std::cout.flush();
}

/** Read this pilot data from superstar, and store into ourselves */
void slam_backend::read_network(const std::string &read_json)
{
	try {
		json::Object return_json=json::Deserialize(read_json);
		
		json::Array depth_readings = return_json["lidar"]["depth"];
		json::Object location = return_json["location"];
		
		double scale = 25;
		
		double x = location["x"].ToDouble() * 1000.0 / scale;
		double y = location["y"].ToDouble() * 1000.0 / scale;
		double angle(location["angle"].ToDouble() / 180.0 * M_PI);
		
		map_location newLocation = baseLocation + map_location(x, y, angle);
		
		if(std::abs(newLocation.get_direction() - robotLocation.get_direction()) < 0.005){
			std::vector<double> converted_depth_readings;
		
			for(auto & depth : depth_readings)
			{
				converted_depth_readings.push_back(depth.ToDouble() / scale);
			}
			
			try {
				map.update(newLocation, converted_depth_readings);
			}
			catch(std::exception& error)
			{
				std::cout<<"Error! "<<error.what()<<std::endl;
			}
		}
		
		robotLocation = newLocation;
		
		//std::cout << depth_readings.size() << "\n";

	} catch (std::exception &e) {

		printf("Exception while processing network JSON: %s\n",e.what());
		printf("   Network data: %ld bytes, '%s'\n", (long)read_json.size(),read_json.c_str());
		// stop();
	}
}

slam_backend *backend=NULL; // Slam backend object for communication with superstar


/* GRAPHICS */

const int ESCKEY = 27;         // ASCII value of Escape

const int START_WIN_SIZE = 1000;  // Start window width & height (pixels)
const int PULLS_PER_SECOND = 5;

double savetime;               // Time of previous movement (sec)

// Draws the occupancy map as a grid of filled squares
void drawGrid(const occupancy_grid & map)
{
	glPushMatrix();
	glBegin(GL_QUADS);
   	for(std::size_t y = map.size(); y > 0; y--)
	{
		for(std::size_t x = 0; x < map.size(); x++)
		{
			double weight = 1.0 - map(x,y-1);
			glColor3f(weight, weight, weight);
			glVertex2d(-1.0 + x, -1.0 + y);
			glVertex2d( 1.0 + x, -1.0 + y);
			glVertex2d( 1.0 + x,  1.0 + y);
			glVertex2d(-1.0 + x,  1.0 + y);
		}
	}
	glEnd();
	glPopMatrix();
}

// Draws the robot as a simple triangle oriented as the robot is
void drawRobot(const map_location & robot)
{
	glPushMatrix();
	double x = robot.get_x();
	double y = robot.get_y();
	glTranslated(x, y, 0.0);
	glRotated(robot.get_direction()*180/M_PI, 0.0,0.0,1.0);
	glBegin(GL_TRIANGLES);
		glColor3f(0.4, 0.1, 0.0);
		glVertex2d(3.0, 0.0);
		glVertex2d(-3.0, 1.5);
		glVertex2d(-3.0,  -1.5);
	glEnd();
	glPopMatrix();
}


// Draw window, text, and display occupancy map
void myDisplay()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Initial transformation
	glLoadIdentity();

	// Draw map and robot in a 2x2 square centered at the origin
	glPushMatrix();
	glTranslated(-1.0, -1.0, 0.);   // Translate to center our grid
	glScaled(2.0/(double)map.size(), 2.0/(double)map.size(), 1.0);  // Scale grid to 2x2 square
	drawGrid(map);
	drawRobot(robotLocation);
	glPopMatrix();

	// Draw documentation
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);  // Set up simple ortho projection
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1., 1., -1., 1.);
	glColor3d(1., 1., 1.);        // Black text
	BitmapPrinter p(-0.9, 0.9, 0.1);
	p.print("Esc      Quit");
	glPopMatrix();                // Restore prev projection
	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
}


// Idle function pulls sensor data from superstar
void myIdle()
{
	//timing
	double currtime = glutGet(GLUT_ELAPSED_TIME) / 1000.;
	double elapsedtime = currtime - savetime;

	if(elapsedtime > 1.0/PULLS_PER_SECOND)
	{
		savetime = currtime;
		// Pull network and redisplay
		try
		{
			backend->do_network();
		}
		catch(std::exception& error)
		{
			std::cout<<"ERROR! "<<error.what()<<std::endl;
			exit(1);
		}
		
		glutPostRedisplay();
	}
}


// GLUT keyboard function
void myKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ESCKEY:  //esc: quit
		exit(0);
		break;
	}
}


// Initialize GL states and global data
void init()
{
	//init opengl
	int i = 0;
	char * c;
	glutInit(&i,&c);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	//generate window
	glutInitWindowSize(START_WIN_SIZE, START_WIN_SIZE);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Robot Mapping");	
	
	//timing
	savetime = glutGet(GLUT_ELAPSED_TIME) / 1000.;

	//transformation stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1., 1., -1., 1.);

	//return to model/view mode
	glMatrixMode(GL_MODELVIEW);
	
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);
	glutKeyboardFunc(myKeyboard);

	//the loop
	glutMainLoop();
}

/* MAIN */

int main(int argc, char *argv[])
{
	map = occupancy_grid(300, 0.5, 0.4, 0.8);
	baseLocation = map_location(map.size()/2, map.size()/2, 0.0);
	robotLocation = map_location(map.size()/2, map.size()/2, 0.0);
	
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
		config.from_cli(argc, argv);

		std::cout << "Connecting to superstar at " << config.get("superstar") << std::endl;

		backend=new slam_backend(config.get("superstar"), config.get("robot"));
		
		init();	// Initialize GUI
	}

	catch(std::exception& error)
	{
		std::cout<<"ERROR! "<<error.what()<<std::endl;
		return 1;
	}

	return 0;
}
