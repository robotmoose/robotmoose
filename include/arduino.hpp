#ifndef ARDUINO_HPP
#define ARDUINO_HPP

#include <mutex>
#include <msl/serial.hpp>
#include <string>

class arduino_t
{
	public:
		arduino_t(const std::string& serial,const size_t baud,const bool show_debug=true);
		arduino_t(const arduino_t& copy)=delete;
		~arduino_t();
		arduino_t& operator=(const arduino_t& copy)=delete;

		bool good() const;
		void start(const bool detach=true);
		void stop();

		void write(const std::string& buffer);

		std::string get_serial() const;
		void set_serial(const std::string& webroot,const bool restart=false);

		size_t get_baud() const;
		void set_baud(const size_t baud,const bool restart=false);

		void show_debug(const bool show);

	private:
		std::mutex write_lock_m;
		void arduino_thread_func_m();
		std::string serial_m;
		size_t baud_m;
		msl::serial_t arduino_m;
		bool show_debug_m;
};

#endif