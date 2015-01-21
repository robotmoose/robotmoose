#ifndef littlestar_HPP
#define littlestar_HPP

#include <cstdint>
#include <functional>
#include <json.h>
#include <mongoose/mongoose.h>
#include <string>

class littlestar_client_t
{
	public:
		littlestar_client_t(const json::Object& json,mg_connection* connection);
		void reply() const;

		json::Object request;
		json::Object response;

	private:
		mg_connection* connection_m;
};

class littlestar_t
{
	public:
		typedef std::function<bool(littlestar_client_t)> client_func_t;

		littlestar_t(client_func_t client_func,const uint16_t port,const std::string webroot="web");
		littlestar_t(const littlestar_t& copy)=delete;
		~littlestar_t();
		littlestar_t& operator=(const littlestar_t& copy)=delete;

		bool good() const;
		void start(const bool detach=false);
		void stop();

		uint16_t get_port() const;
		void set_port(const uint16_t port,const bool restart=false);

		std::string get_webroot() const;
		void set_webroot(const std::string& webroot,const bool restart=false);


	private:
		static int client_func_handler(mg_connection* connection,enum mg_event event);
		void server_thread_func_m();
		mg_server* server_m;
		client_func_t client_func_m;
		uint16_t port_m;
		std::string webroot_m;
};

#endif