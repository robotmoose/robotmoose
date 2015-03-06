//Windows Dependencies:
//		-lWs2_32

#ifndef MSL_C11_SOCKET_HPP
#define MSL_C11_SOCKET_HPP

#include <cstdint>
#include <string>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
	#include <winsock2.h>
	#if(!defined(socklen_t))
		typedef int socklen_t;
	#endif
#else
	#include <netinet/in.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif

namespace msl
{
	typedef int socket_fd_t;

	struct socket_device_t
	{
		socket_fd_t fd;
		sockaddr_in ip_bind;
		sockaddr_in ip_connect;
		bool host;
		bool tcp;
		size_t buffer_size;
	};

	class socket_t
	{
		public:
			socket_t(const std::string& ip,const bool tcp,const size_t buffer_size=200);
			virtual void open();
			virtual void close();
			virtual bool good() const;
			virtual ssize_t available() const;
			virtual ssize_t read(void* buf,const size_t count) const;
			virtual ssize_t write(const void* buf,const size_t count) const;
			virtual ssize_t write(const std::string& buf) const;
			virtual socket_t accept() const;
			virtual std::string address() const;
			size_t buffer_size() const;

		private:
			socket_device_t device_m;
	};

	class tcp_socket_t:public socket_t
	{
		public:
			tcp_socket_t(const std::string& ip);
	};

	class udp_socket_t:public socket_t
	{
		public:
			udp_socket_t(const std::string& ip,const size_t buffer_size=200);
	};
}

#endif