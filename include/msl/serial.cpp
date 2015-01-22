#include "serial.hpp"

static bool valid_baud(const size_t baud)
{
	return (baud==300||baud==1200||baud==2400||baud==4800||baud==9600||
		baud==19200||baud==38400||baud==57600||baud==115200);
}

#if(defined(_WIN32)&&!defined(__CYGWIN__))

static ssize_t read(msl::serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_read=-1;
	if(ReadFile(fd,buf,count,&bytes_read,0))
		return bytes_read;
	return -1;
}

static ssize_t write(msl::serial_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_sent=-1;
	if(WriteFile(fd,buf,count,&bytes_sent,0))
		return bytes_sent;
	return -1;
}

static ssize_t select(msl::serial_device_t device)
{
	COMSTAT port_stats;
	DWORD error_flags=0;

	if(ClearCommError(device.fd,&error_flags,&port_stats))
		return port_stats.cbInQue;

	return -1;
}

static void serial_close(const msl::serial_device_t& device)
{
	CloseHandle(device.fd);
}

static msl::serial_device_t serial_open(const std::string& name,const size_t baud)
{
	msl::serial_device_t device{INVALID_HANDLE_VALUE,name,baud};
	std::string full_path="\\\\.\\"+device.name;
	device.fd=CreateFile(full_path.c_str(),GENERIC_READ|GENERIC_WRITE,
		0,0,OPEN_EXISTING,0,nullptr);
	DCB options;

	if(device.fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&GetCommState(device.fd,&options))
	{
		options.BaudRate=baud;
		options.fParity=FALSE;
		options.Parity=NOPARITY;
		options.StopBits=ONESTOPBIT;
		options.ByteSize=8;
		options.fOutxCtsFlow=FALSE;
		options.fOutxDsrFlow=FALSE;
		options.fDtrControl=DTR_CONTROL_DISABLE;
		options.fRtsControl=RTS_CONTROL_DISABLE;

		if(SetCommState(device.fd,&options)&&EscapeCommFunction(device.fd,CLRDTR|CLRRTS))
			return device;
	}

	serial_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool serial_valid(const msl::serial_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	std::string full_path="\\\\.\\"+device.name;
	msl::serial_fd_t fd=CreateFile(full_path.c_str(),GENERIC_READ,0,0,OPEN_EXISTING,0,nullptr);
	bool valid=fd!=INVALID_HANDLE_VALUE||GetLastError()!=ERROR_FILE_NOT_FOUND;
	CloseHandle(fd);

	return valid;
}

std::vector<std::string> msl::serial_t::list()
{
	std::vector<std::string> list;

	for(unsigned int ii=1;ii<=256;++ii)
	{
		std::string full_path="\\\\.\\com"+std::to_string(ii);
		msl::serial_fd_t fd=CreateFile(full_path.c_str(),GENERIC_READ,0,0,OPEN_EXISTING,0,nullptr);

		if(fd!=INVALID_HANDLE_VALUE||GetLastError()!=ERROR_FILE_NOT_FOUND)
			list.push_back("com"+std::to_string(ii));

		CloseHandle(fd);
	}

	return list;
}

#else

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE (-1)

//Copyright 2010, Paul Stoffregen(paul@pjrc.com)
std::vector<std::string> serial_prefixes
{
	"S",	// "normal" SerialPort Ports - MANY drivers using this
	"USB",	// USB to serial converters
	"ACM",	// USB serial modem, CDC class, Abstract Control Model
	"MI",	// MOXA Smartio/Industio family multiport serial... nice card, I have one :-)
	"MX",	// MOXA Intellio family multiport serial
	"C",	// Cyclades async multiport, no longer available, but I have an old ISA one! :-)
	"D",	// Digiboard (still in 2.6 but no longer supported), new Moschip MCS9901
	"P",	// Hayes ESP serial cards (obsolete)
	"M",	// PAM Software's multimodem & Multitech ISI-Cards
	"E",	// Stallion intelligent multiport (no longer made)
	"L",	// RISCom/8 multiport serial
	"W",	// specialix IO8+ multiport serial
	"X",	// Specialix SX series cards, also SI & XIO series
	"SR",	// Specialix RIO serial card 257+
	"n",	// Digi International Neo (yes lowercase 'n', drivers/serial/jsm/jsm_driver.c)
	"FB",	// serial port on the 21285 StrongArm-110 core logic chip
	"AM",	// ARM AMBA-type serial ports (no DTR/RTS)
	"AMA",	// ARM AMBA-type serial ports (no DTR/RTS)
	"AT",	// Atmel AT91 / AT32 SerialPort ports
	"BF",	// Blackfin 5xx serial ports (Analog Devices embedded DSP chips)
	"CL",	// CLPS711x serial ports (ARM processor)
	"A",	// ICOM SerialPort
	"SMX",	// Motorola IMX serial ports
	"SOIC",	// ioc3 serial
	"IOC",	// ioc4 serial
	"PSC",	// Freescale MPC52xx PSCs configured as UARTs
	"MM",	// MPSC (UART mode) on Marvell GT64240, GT64260, MV64340...
	"B",	// Mux console found in some PA-RISC servers
	"NX",	// NetX serial port
	"PZ",	// PowerMac Z85c30 based ESCC cell found in the "macio" ASIC
	"SAC",	// Samsung S3C24XX onboard UARTs
	"SA",	// SA11x0 serial ports
	"AM",	// KS8695 serial ports & Sharp LH7A40X embedded serial ports
	"TX",	// TX3927/TX4927/TX4925/TX4938 internal SIO controller
	"SC",	// Hitachi SuperH on-chip serial module
	"SG",	// C-Brick SerialPort Port (and console) SGI Altix machines
	"HV",	// SUN4V hypervisor console
	"UL",	// Xilinx uartlite serial controller
	"VR",	// NEC VR4100 series SerialPort Interface Unit
	"CPM",	// CPM (SCC/SMC) serial ports; core driver
	"Y",	// Amiga A2232 board
	"SL",	// Microgate SyncLink ISA and PCI high speed multiprotocol serial
	"SLG",	// Microgate SyncLink GT (might be sync HDLC only?)
	"SLM",	// Microgate SyncLink Multiport high speed multiprotocol serial
	"CH",	// Chase Research AT/PCI-Fast serial card
	"F",	// Computone IntelliPort serial card
	"H",	// Chase serial card
	"I",	// virtual modems
	"R",	// Comtrol RocketPort
	"SI",	// SmartIO serial card
	"T",	// Technology Concepts serial card
	"V",	// Comtrol VS-1000 serial controller
};

static ssize_t select(msl::serial_device_t device)
{
	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(device.fd,&rfds);
	return select(device.fd+1,&rfds,nullptr,nullptr,&temp);
}

static speed_t baud_rate(const size_t baud)
{
	if(baud==300)
		return B300;
	else if(baud==1200)
		return B1200;
	else if(baud==2400)
		return B2400;
	else if(baud==4800)
		return B4800;
	else if(baud==9600)
		return B9600;
	else if(baud==19200)
		return B19200;
	else if(baud==38400)
		return B38400;
	else if(baud==57600)
		return B57600;
	else if(baud==115200)
		return B115200;

	return B0;
}

static void serial_close(const msl::serial_device_t& device)
{
	close(device.fd);
}

static msl::serial_device_t serial_open(const std::string& name,const size_t baud)
{
	msl::serial_device_t device{INVALID_HANDLE_VALUE,name,baud};

	device.fd=open(device.name.c_str(),O_RDWR|O_NOCTTY|O_SYNC);
	termios options;

	if(device.fd!=INVALID_HANDLE_VALUE&&valid_baud(baud)&&tcgetattr(device.fd,&options)!=-1&&
		cfsetispeed(&options,baud_rate(baud))!=-1&&cfsetospeed(&options,baud_rate(baud))!=-1)
	{
		options.c_cflag|=(CS8|CLOCAL|CREAD|HUPCL);
		options.c_iflag|=(IGNBRK|IGNPAR);
		options.c_iflag&=~(IXON|IXOFF|IXANY);
		options.c_lflag=0;
		options.c_oflag=0;
		options.c_cc[VMIN]=0;
		options.c_cc[VTIME]=1;
		options.c_cflag&=~(PARENB|PARODD);
		options.c_cflag&=~CSTOPB;
		options.c_cflag&=~CRTSCTS;

		if(tcsetattr(device.fd,TCSANOW,&options)!=-1&&tcflush(device.fd,TCIFLUSH)!=-1&&
			tcdrain(device.fd)!=-1)
			return device;
	}

	serial_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool serial_valid(const msl::serial_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	termios options;
	return tcgetattr(device.fd,&options)!=-1;
}

std::vector<std::string> msl::serial_t::list()
{
	std::vector<std::string> list;
	std::vector<std::string> files;

	DIR* dp=opendir("/dev");

	while(dp!=nullptr)
	{
		dirent* np=readdir(dp);

		if(np==nullptr)
		{
			closedir(dp);
			break;
		}

		std::string node_name(np->d_name);

		if(node_name!="."&&node_name!=".."&&np->d_type==DT_CHR)
			files.push_back(node_name);
	}

	for(auto file:files)
	{
		for(auto prefix:serial_prefixes)
		{
			if(file.find("tty"+prefix)==0)
			{
				msl::serial_fd_t fd=::open(std::string("/dev/"+file).c_str(),O_RDWR|O_NOCTTY);
				termios options;

				if(fd!=INVALID_HANDLE_VALUE&&tcgetattr(fd,&options)!=-1)
					list.push_back({"/dev/"+file});

				::close(fd);
				break;
			}
		}
	}

	return list;
}

#endif

static ssize_t serial_available(const msl::serial_device_t& device)
{
	if(!serial_valid(device))
		return -1;

	return select(device);
}

static ssize_t serial_read(const msl::serial_device_t& device,void* buffer,const size_t size)
{
	if(!serial_valid(device))
		return -1;

	return read(device.fd,(char*)buffer,size);
}

static ssize_t serial_write(const msl::serial_device_t& device,const void* buffer,const size_t size)
{
	if(!serial_valid(device))
		return -1;

	return write(device.fd,(char*)buffer,size);
}

msl::serial_t::serial_t(const std::string& name,const size_t baud):device_m{INVALID_HANDLE_VALUE,name,baud}
{}

msl::serial_t::~serial_t()
{
	close();
}

void msl::serial_t::open(const std::string& name,const size_t baud)
{
	close();
	device_m={INVALID_HANDLE_VALUE,name,baud};
	open();
}

void msl::serial_t::open()
{
	close();
	device_m=serial_open(device_m.name,device_m.baud);
}

void msl::serial_t::close()
{
	serial_close(device_m);
}

bool msl::serial_t::good() const
{
	return serial_valid(device_m);
}

ssize_t msl::serial_t::available() const
{
	return serial_available(device_m);
}

ssize_t msl::serial_t::read(void* buf,const size_t count) const
{
	return serial_read(device_m,buf,count);
}

ssize_t msl::serial_t::write(const void* buf,const size_t count) const
{
	return serial_write(device_m,buf,count);
}

ssize_t msl::serial_t::write(const std::string& buf) const
{
	return serial_write(device_m,buf.c_str(),buf.size());
}

std::string msl::serial_t::name() const
{
	return device_m.name;
}

size_t msl::serial_t::baud() const
{
	return device_m.baud;
}