#include "cereal.hpp"

cereal_device_t::cereal_device_t(const cereal_fd_t fd,const std::string& name,const size_t baud):
	fd(fd),name(name),baud(baud)
{}

static bool valid_baud(const size_t baud)
{
	return (baud==300||baud==1200||baud==2400||baud==4800||baud==9600||
		baud==19200||baud==38400||baud==57600||baud==115200);
}

#if(defined(_WIN32)&&!defined(__CYGWIN__))

static int read(cereal_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_read=-1;
	if(ReadFile(fd,buf,count,&bytes_read,0))
		return bytes_read;
	return -1;
}

static int write(cereal_fd_t fd,void* buf,size_t count)
{
	DWORD bytes_sent=-1;
	if(WriteFile(fd,buf,count,&bytes_sent,0))
		return bytes_sent;
	return -1;
}

static int select(cereal_device_t device)
{
	COMSTAT port_stats;
	DWORD error_flags=0;

	if(ClearCommError(device.fd,&error_flags,&port_stats))
		return port_stats.cbInQue;

	return -1;
}

static void cereal_close(const cereal_device_t& device)
{
	CloseHandle(device.fd);
}

static cereal_device_t cereal_open(const std::string& name,const size_t baud)
{
	cereal_device_t device{INVALID_HANDLE_VALUE,name,baud};
	std::string full_path="\\\\.\\"+device.name;
	device.fd=CreateFile(full_path.c_str(),GENERIC_READ|GENERIC_WRITE,
		0,0,OPEN_EXISTING,0,NULL);
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

	cereal_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool cereal_valid(const cereal_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	std::string full_path="\\\\.\\"+device.name;
	cereal_fd_t fd=CreateFile(full_path.c_str(),GENERIC_READ,0,0,OPEN_EXISTING,0,NULL);
	bool valid=fd!=INVALID_HANDLE_VALUE||GetLastError()!=ERROR_FILE_NOT_FOUND;
	CloseHandle(fd);

	return valid;
}

#else

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE (-1)

static int select(cereal_device_t device)
{
	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(device.fd,&rfds);
	return select(device.fd+1,&rfds,NULL,NULL,&temp);
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

static void cereal_close(const cereal_device_t& device)
{
	close(device.fd);
}

static cereal_device_t cereal_open(const std::string& name,const size_t baud)
{
	cereal_device_t device(INVALID_HANDLE_VALUE,name,baud);

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

	cereal_close(device);
	device.fd=INVALID_HANDLE_VALUE;
	return device;
}

static bool cereal_valid(const cereal_device_t& device)
{
	if(device.fd==INVALID_HANDLE_VALUE||!valid_baud(device.baud))
		return false;

	termios options;
	return tcgetattr(device.fd,&options)!=-1;
}

#endif

static int cereal_available(const cereal_device_t& device)
{
	if(!cereal_valid(device))
		return -1;

	return select(device);
}

static int cereal_read(const cereal_device_t& device,void* buffer,const size_t size)
{
	if(!cereal_valid(device))
		return -1;

	return read(device.fd,(char*)buffer,size);
}

static int cereal_write(const cereal_device_t& device,const void* buffer,const size_t size)
{
	if(!cereal_valid(device))
		return -1;

	return write(device.fd,(char*)buffer,size);
}

cereal_t::cereal_t(const std::string& name,const size_t baud):device_m(INVALID_HANDLE_VALUE,name,baud)
{}

cereal_t::~cereal_t()
{
	close();
}

void cereal_t::open(const std::string& name,const size_t baud)
{
	close();
	device_m=cereal_device_t(INVALID_HANDLE_VALUE,name,baud);
	open();
}

void cereal_t::open()
{
	close();
	device_m=cereal_open(device_m.name,device_m.baud);
}

void cereal_t::close()
{
	cereal_close(device_m);
}

bool cereal_t::good() const
{
	return cereal_valid(device_m);
}

int cereal_t::available() const
{
	return cereal_available(device_m);
}

int cereal_t::read(void* buf,const size_t count) const
{
	return cereal_read(device_m,buf,count);
}

int cereal_t::write(const void* buf,const size_t count) const
{
	return cereal_write(device_m,buf,count);
}

int cereal_t::write(const std::string& buf) const
{
	return cereal_write(device_m,buf.c_str(),buf.size());
}

std::string cereal_t::name() const
{
	return device_m.name;
}

size_t cereal_t::baud() const
{
	return device_m.baud;
}