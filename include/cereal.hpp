#ifndef CEREAL_HPP
#define CEREAL_HPP

#include <string>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
	#include <conio.h>
	#include <stdio.h>
	#include <windows.h>
	#include <commctrl.h>
	typedef HANDLE cereal_fd_t;
#else
	typedef int cereal_fd_t;
#endif

class cereal_device_t
{
	public:
		cereal_fd_t fd;
		std::string name;
		size_t baud;

		cereal_device_t(const cereal_fd_t fd,const std::string& name,const size_t baud);
};

class cereal_t
{
	public:
		cereal_t(const std::string& name,const size_t baud);
		~cereal_t();
		void open(const std::string& name,const size_t baud);
		void open();
		void close();
		bool good() const;
		int available() const;
		int read(void* buf,const size_t count) const;
		int write(const void* buf,const size_t count) const;
		int write(const std::string& buf) const;
		std::string name() const;
		size_t baud() const;

	private:
		cereal_t(const cereal_t& copy);
		cereal_t& operator=(const cereal_t& copy);
		cereal_device_t device_m;
};

#endif