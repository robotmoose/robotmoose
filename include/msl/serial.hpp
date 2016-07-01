#ifndef MSL_C11_SERIAL_HPP
#define MSL_C11_SERIAL_HPP

#include <string>
#include <vector>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
	#include <conio.h>
	#include <stdio.h>
	#include <windows.h>
	#include <commctrl.h>
#endif

namespace msl
{
	#if(defined(_WIN32)&&!defined(__CYGWIN__))
		typedef HANDLE serial_fd_t;
	#else
		typedef ssize_t serial_fd_t;
	#endif

	struct serial_device_t
	{
		msl::serial_fd_t fd;
		std::string name;
		size_t baud;
	};

	class serial_t
	{
		public:
			static std::vector<std::string> list();

			serial_t(const std::string& name,const size_t baud);
			serial_t(const serial_t& copy)=delete;
			~serial_t();
			serial_t& operator=(const serial_t& copy)=delete;
			void open(const std::string& name,const size_t baud);
			void open();
			void close();
			bool good() const;
			ssize_t available() const;
			ssize_t read(void* buf,const size_t count) const;
			ssize_t write(const void* buf,const size_t count) const;
			ssize_t write(const std::string& buf) const;
			std::string name() const;
			size_t baud() const;

		private:
			serial_device_t device_m;
	};
}

#endif