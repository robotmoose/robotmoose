//Windows Dependencies:
//		-lWs2_32

#include "socket.hpp"

#include <stdexcept>
#include <cstring>

#define INVALID_SOCKET_VALUE (~0)

#if(!defined(_WIN32)||defined(__CYGWIN__))
	#include <signal.h>
#else
	int close(unsigned int fd)
	{
		return closesocket(fd);
	}
#endif

static size_t parse_integer(const std::string& str,const size_t pos,int& integer)
{
	for(size_t ii=pos;ii<str.size();++ii)
	{
		if(isdigit(str[ii])==0)
		{
			if(ii-pos==0)
				break;

			integer=std::stoi(str.substr(pos,ii-pos));
			return ii;
		}

		if(isdigit(str[ii])!=0&&ii==str.size()-1)
		{
			integer=std::stoi(str.substr(pos,ii-pos+1));
			return ii+1;
		}
	}

	return std::string::npos;
}

static bool parse_symbol(const std::string& str,const size_t pos,const char sym)
{
	return (pos<str.size()&&str[pos]==sym);
}

static void string_to_rawaddr(std::string str,uint8_t* ip_bind,uint16_t& port_bind,bool& host,
	uint8_t* ip_connect,uint16_t& port_connect)
{
	int temp_ip[8];
	int temp_port[2];
	bool temp_host=false;
	size_t pos=0;
	bool error=false;

	memset(temp_ip,0,4*8);
	memset(temp_port,0,4*2);

	for(int ii=0;ii<2;++ii)
	{
		for(int jj=0;jj<4;++jj)
		{
			pos=parse_integer(str,pos,temp_ip[ii*4+jj]);

			if(pos!=std::string::npos)
			{
				if(jj<3&&parse_symbol(str,pos,'.'))
					++pos;
			}
			else
			{
				error=true;
				break;
			}

			if(temp_ip[ii*4+jj]<0||temp_ip[ii*4+jj]>255)
			{
				error=true;
				break;
			}
		}

		if(!error)
		{
			if(parse_symbol(str,pos,':'))
			{
				++pos;
				pos=parse_integer(str,pos,temp_port[ii]);
				error=(pos==std::string::npos);

				if(!error&&(temp_port[ii]<0||temp_port[ii]>65535))
				{
					error=true;
					break;
				}
			}
		}

		if(!error&&ii==0)
		{
			if(parse_symbol(str,pos,'<')||parse_symbol(str,pos,'>'))
			{
				temp_host=parse_symbol(str,pos,'<');
				++pos;
			}
			else
			{
				error=true;
			}
		}

		if(error)
			break;
	}

	if(error)
		throw std::invalid_argument("string_to_rawaddr");

	for(int ii=0;ii<4;++ii)
		ip_bind[ii]=temp_ip[ii];

	port_bind=temp_port[0];

	host=temp_host;

	for(int ii=0;ii<4;++ii)
		ip_connect[ii]=temp_ip[4+ii];

	port_connect=temp_port[1];
}

static bool socket_inited=false;

static void socket_init()
{
	if(!socket_inited)
	{
		socket_inited=true;

		#if(defined(_WIN32)&&!defined(__CYGWIN__))
			WSADATA temp;
			WSAStartup(0x0002,&temp);
		#else
			signal(SIGPIPE,SIG_IGN);
		#endif
	}
}

static void socket_close(msl::socket_device_t& device)
{
	close(device.fd);
	device.fd=INVALID_SOCKET_VALUE;
}

static void socket_open(msl::socket_device_t& device)
{
	socket_init();

	int type=SOCK_STREAM;

	if(!device.tcp)
		type=SOCK_DGRAM;

	device.fd=socket(AF_INET,type,0);
	socklen_t ip_length=sizeof(socklen_t);

	if(device.fd!=INVALID_SOCKET_VALUE)
	{
		linger lingerer;
		lingerer.l_onoff=1;
		lingerer.l_linger=10;
		int on=1;

		if(setsockopt(device.fd,SOL_SOCKET,SO_LINGER,(const char*)&lingerer,sizeof(lingerer))!=0)
			socket_close(device);
		if(setsockopt(device.fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on))!=0)
			socket_close(device);

		if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_RCVBUF,(const char*)&device.buffer_size,ip_length)!=0)
				socket_close(device);
		if(!device.tcp&&setsockopt(device.fd,SOL_SOCKET,SO_SNDBUF,(const char*)&device.buffer_size,ip_length)!=0)
			socket_close(device);

		if(bind(device.fd,(sockaddr*)&device.ip_bind,sizeof(device.ip_bind))!=0)
			socket_close(device);

		if(device.host&&device.tcp)
		{
			if(listen(device.fd,16)!=0)
				socket_close(device);
		}
		else
		{
			if(connect(device.fd,(sockaddr*)&device.ip_connect,sizeof(device.ip_connect))!=0)
				socket_close(device);
		}

		if(getsockname(device.fd,(sockaddr*)&device.ip_bind,&ip_length)!=0)
			socket_close(device);
	}
}

static ssize_t socket_available(const msl::socket_device_t& device)
{
	socket_init();

	if(device.fd==INVALID_SOCKET_VALUE)
		return -1;

	timeval temp={0,0};
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET((unsigned int)device.fd,&rfds);

	return select(1+device.fd,&rfds,nullptr,nullptr,&temp);
}

static ssize_t socket_read(const msl::socket_device_t& device,void* buffer,const size_t size,const int flags=0)
{
	socket_init();
	return recv(device.fd,(char*)buffer,size,flags);
}

static ssize_t socket_write(const msl::socket_device_t& device,const void* buffer,const size_t size,const int flags=0)
{
	socket_init();
	return send(device.fd,(char*)buffer,size,flags);
}

static bool socket_valid(const msl::socket_device_t& device)
{
	socket_init();

	if(device.fd==INVALID_SOCKET_VALUE)
		return false;

	char temp;

	if(socket_available(device)>0&&socket_read(device,&temp,1,MSG_PEEK)==0)
		return false;

	return (socket_available(device)>=0);
}

static msl::socket_device_t socket_accept(const msl::socket_device_t& device)
{
	socket_init();
	msl::socket_device_t client{INVALID_SOCKET_VALUE,{},device.ip_connect,false,device.tcp,device.buffer_size};

	if(socket_available(device)>0)
	{
		socklen_t ip_length=sizeof(socklen_t);
		client.fd=accept(device.fd,(sockaddr*)&client.ip_bind,&ip_length);

		if(socket_valid(client)&&getsockname(client.fd,(sockaddr*)&client.ip_connect,&ip_length)!=0)
			socket_close(client);
	}

	return client;
}

msl::socket_t::socket_t(const std::string& ip,const bool tcp,const size_t buffer_size)
{
	bool host;
	uint8_t ip_bind[4];
	uint8_t ip_connect[4];
	uint16_t port_bind;
	uint16_t port_connect;

	string_to_rawaddr(ip,ip_bind,port_bind,host,ip_connect,port_connect);
	device_m.fd=INVALID_SOCKET_VALUE;

	device_m.ip_bind.sin_family=AF_INET;
	memcpy(&device_m.ip_bind.sin_addr,ip_bind,4);
	device_m.ip_bind.sin_port=htons(port_bind);

	device_m.ip_connect.sin_family=AF_INET;
	memcpy(&device_m.ip_connect.sin_addr,ip_connect,4);
	device_m.ip_connect.sin_port=htons(port_connect);

	device_m.buffer_size=buffer_size;
	device_m.host=host;
	device_m.tcp=tcp;
}

void msl::socket_t::open()
{
	socket_open(device_m);
}

void msl::socket_t::close()
{
	socket_close(device_m);
}

bool msl::socket_t::good() const
{
	return socket_valid(device_m);
}

ssize_t msl::socket_t::available() const
{
	return socket_available(device_m);
}

ssize_t msl::socket_t::read(void* buf,const size_t count) const
{
	return socket_read(device_m,buf,count);
}

ssize_t msl::socket_t::write(const void* buf,const size_t count) const
{
	return socket_write(device_m,buf,count);
}

ssize_t msl::socket_t::write(const std::string& buf) const
{
	return socket_write(device_m,buf.c_str(),buf.size());
}

msl::socket_t msl::socket_t::accept() const
{
	msl::socket_t client("0.0.0.0:0>0.0.0.0:0",true);
	client.device_m=socket_accept(device_m);
	return client;
}

std::string msl::socket_t::address() const
{
	std::string address="";

	for(int ii=0;ii<4;++ii)
	{
		address+=std::to_string((unsigned int)((unsigned char*)(&device_m.ip_bind.sin_addr))[ii]);

		if(ii!=3)
			address+='.';
	}

	address+=":"+std::to_string(ntohs(device_m.ip_bind.sin_port));

	if(device_m.host)
		address+="<";
	else
		address+=">";

	for(int ii=0;ii<4;++ii)
	{
		address+=std::to_string((unsigned int)((unsigned char*)(&device_m.ip_connect.sin_addr))[ii]);

		if(ii!=3)
			address+='.';
	}

	address+=":"+std::to_string(ntohs(device_m.ip_connect.sin_port));

	return address;
}

size_t msl::socket_t::buffer_size() const
{
	return device_m.buffer_size;
}

msl::tcp_socket_t::tcp_socket_t(const std::string& ip):socket_t(ip,true)
{}

msl::udp_socket_t::udp_socket_t(const std::string& ip,const size_t buffer_size):socket_t(ip,false,buffer_size)
{}