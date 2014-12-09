/**
  Neato XV-11 Laser Distance Sensor (LDS) display and interface code.
  This version does most of the protocol decoding on the Arduino side.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-19 (Public Domain)
*/
#include <GL/glut.h>
//#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "serial.h"
#include "serial.cpp"
#include "firmware/serial_packet.h"
#include "firmware/neato_serial.h"


template <class sub>
class serial_logger {
public:
	//std::ofstream log;
	#define log std::cout
	sub &serial;
	int last_avail;
	serial_logger(sub &serial_) :serial(serial_) {
		//log.open("serial.log");
		log<<"Opened serial port\n";
		last_avail=-1;
	}
	
	void begin(int baudrate) { serial.begin(baudrate); }
	
	/// Return 1 if data is available for reading.
	virtual int available(void) {
		int a=serial.available();
		if (a!=last_avail) {
			log<<(a?"A":"a")<<"\n"; log.flush();
			last_avail=a;
		}
		return a;
	}
	/// Return a read byte, or -1 if no byte is ready.
	virtual int read(void) {
		int c=serial.read();
		log<<"read "<<std::hex<<c<<"\n"; log.flush();
		return c;
	}
	
	/// Send a byte out the serial port.
	virtual void write(unsigned char c) {
		log<<"write "<<std::hex<<(int)c<<"\n"; log.flush();
		serial.write(c);
		
	}
	/// Send a block of bytes out the serial port
	virtual void write(const unsigned char *data,int length) {
		for (int i=0;i<length;i++) write(data[i]);
	}
	
};

/**
  This class parses the serial output from the Arduino,
  and stores the output as a set of distances in directions.
*/
class NeatoLDSreceiver {
public:
	
	enum {ndir=360}; // total number of distance reports: one per direction
	NeatoLDSdir dir[ndir];
	int speed64;

#define SERIAL_LOGGING 1

#if SERIAL_LOGGING
	SerialPort realport;
	serial_logger<SerialPort> port;
	A_packet_formatter<serial_logger<SerialPort> > pkt;
#else
	SerialPort port;
	A_packet_formatter<SerialPort> pkt;
#endif
	
	NeatoLDSreceiver() 
		:
#if SERIAL_LOGGING
		realport(), port(realport), 
#endif
		pkt(port)
	{
		speed64=0;
		port.begin(57600);
		memset(dir,0,sizeof(dir));
		
		printf("Waiting for LDS to spin up and start sending data\n"); fflush(stdout);
		while (port.available()==0) {}
		printf("Waiting first packet to arrive\n"); fflush(stdout);
		A_packet ping;
		while (1!=pkt.read_packet(ping)) {}
		printf("Robot sends packet %d (%d bytes).  Beginning decode.\n",
			ping.command,ping.length); fflush(stdout);
	}
	
	void read(void) {
	// Request distance report
		pkt.write_packet(0xD,0,NULL);
		
	// Wait for report to come back
		A_packet p;
		int leash=1000000;
		while (1) {
			int n=pkt.read_packet(p);
			if (n==1) break;
			if (n == 0);//sleep(0);
			// else n==-1
			leash--;
			if (leash%1000==0) {
#if SERIAL_LOGGING
				printf("."); fflush(stdout);
#endif
				if (leash<=0) { printf("Read error...\n"); return; }
			}
		}
		
		NeatoLDSbatch b;
		if (p.command!=0xD) {
			printf("Error on read Packet: command %x not expected (length %d)\n",p.command,p.length);
			return;
		}
		if (!p.get(b)) {
			printf("Error on read Packet: %d bytes back\n",p.length);
			return;
		}
		
		printf("Packet: index %d errors %d speed %d distance %d/%d\n",
			b.index, b.errors, b.speed64/64, b.dir[0].distance, b.dir[0].signal);
		
		if (b.index<0 || b.index>=360) printf("Error: spin index %d invalid\n",b.index);
		if (b.errors>0) printf("Error: serial errors detected (%d)\n",b.errors);
		if (b.index>=0) {
			for (int i=0;i<NeatoLDSbatch::size;i++) {
				int d=i+b.index;
				if (d>=0 && d<ndir) dir[d]=b.dir[i];
			}
		}
	}
	
	void print(void) {
		printf("\x1B[0;0f"); // force cursor to home (VT100)
		printf("Speed: %.1f rpm\n",speed64*(1.0/64.0));
		
		for (int row=0;row<36;row++) {
			printf("%3ddeg: ",row*10);
			for (int col=0;col<10;col++) {
				//const char *qual="X?.";
				int d=10*row+col;
				printf("%4d %1d  ",
					dir[d].distance,dir[d].signal); // ,qual[dir[d].quality]);
			}
			printf("\n");
		}
		fflush(stdout);
	}
};


NeatoLDSreceiver n;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	n.read();
	// n.print();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glColor3f(0,0,0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	float lastRa = 0;
	for (int angle=0;angle<360;angle++) {
		float ra=angle*M_PI/180.0;
		float r=n.dir[angle].distance/5000.0;
		
		float q = (n.dir[angle].signal)/10.0;
		glColor3f(1.0-q,q,0);
		if(r == 0) {
			r = lastRa;
		}
		lastRa = r;
		float upsideDown=+1; // -1 if laser facing floor; +1 if facing ceiling.
		float x=r*cos(ra), y=upsideDown*r*sin(ra);
		glVertex3f(x,y,0);
	}
	glEnd();
	
	glutSwapBuffers();
	glutPostRedisplay();
}


int main(int argc, char *argv[]) {
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(700,700);
	glutCreateWindow("Neato LDS Display");

	glutDisplayFunc(display);
	
	glutMainLoop();
	return 0;
}
