#ifndef SERIAL_HPP

#include "libserialport.h"
#include <string>
#include <vector>

typedef std::vector<std::string> serial_list_t;

class serial_t
{
	public:
		static serial_list_t list();

		serial_t(const std::string& name);
		serial_t(const serial_t& copy);
		serial_t& operator=(const serial_t& copy);
		~serial_t();

		std::string name() const;

		bool good() const;
		bool open(const size_t baud);
		void close();

		size_t available();
		size_t read(void* buffer,const size_t count);
		size_t write(const void* buffer,const size_t count);

	private:
		serial_t(const sp_port* port);
		void free();

		sp_port* port_m;
		bool good_m;
};

#endif