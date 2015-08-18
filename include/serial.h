/**
 This is an Arduino-inspired C++ serial port library.
*/
#ifndef __serial_h__
#define __serial_h__

#include <string>
#include <vector>
#include <stdio.h> /* for error reporting */
#include <string.h> /* for strlen & such reporting */

#if defined(_WIN32)
#  define WINDOWS 1
#  include <windows.h>
#  define snprintf _snprintf /* what the heck, windows... */
#elif defined(__APPLE__)||defined(__MACH__)
#  define MACOSX 1
#  include <termios.h>
#else
#  define LINUX 1
#  include <termios.h>
#endif

class SerialPort
{
public:
	SerialPort();
	~SerialPort();

/* This is the Arduino-inspired interface */
	/// Usage: Serial.begin(115200);
	int begin(int baudrate) {
		std::vector<std::string> ports=port_list();

		if (ports.size()==0) { printf("ERROR!  No serial ports found!  (Is it plugged in?)\n"); return -1; }

		for(size_t ii=0;ii<ports.size();++ii)
		{
			printf("Trying %s...\n",ports[ii].c_str());

			if(0==Open(ports[ii]))
			{
				printf("\nOpened serial port %s, baud rate %d\n",ports[ii].c_str(),baudrate);
				Set_baud(baudrate);
				return 0;
			}
		}

		printf("ERROR!  No serial ports were opened!  (Do you have permissions?)\n");
		return -1;
	}

	/// Return the number of bytes available to read, or 0 if none available.
	int available(void) {
		if (Input_wait(0)>0) return 1;
		else return 0;
	}

	/// Read a byte of serial data, or return -1 if none is available.
	int read(void) {
		if (Input_wait(0)>0) {
			unsigned char c;
			if(Read(&c,1) != 1)
			{
				return -1;
			}

			else return c;
		} else return -1;
	}

	/// Write a byte of serial data
	void write(unsigned char theByte) {
		Write(&theByte,1);
	}

	/// Write a string to the serial port
	void write(char *theString) {
		while (*theString!=0) Write(theString++,1);
	}

	/// Write an entire buffer
	void write(const void *theBuffer,int len) {
		Write(theBuffer,len);
	}


/* This is the original C++ interface. */
	std::vector<std::string> port_list();
	int Open(const std::string& name);
	std::string error_message();
	int Set_baud(int baud);
	int Get_baud() const;
	int Read(void *ptr, int count);
	int Write(const void *ptr, int len);
	int Input_wait(int msec);
	void Input_discard(void);
	int Set_control(int dtr, int rts);
	void Output_flush();
	void Close(void);
	int Is_open(void);
	std::string get_name(void);
private:
	int port_is_open;
	std::string port_name;
	int baud_rate;
	std::string error_msg;
private:
#if defined(LINUX) || defined(MACOSX)
	int port_fd;
	struct termios settings_orig;
	struct termios settings;
#elif defined(WINDOWS)
	HANDLE port_handle;
	COMMCONFIG port_cfg_orig;
	COMMCONFIG port_cfg;
#endif
};

extern SerialPort Serial;

#endif // __serial_h__
