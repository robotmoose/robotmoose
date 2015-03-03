#ifndef MSL_C11_WEBSERVER_HPP
#define MSL_C11_WEBSERVER_HPP

#include <functional>
#include <string>

#include "mongoose/mongoose.h"

namespace msl
{
	class webserver_t
	{
		public:
			typedef std::function<int(const mg_connection& connection,enum mg_event event)> client_func_t;

			webserver_t(client_func_t client_func,const std::string& address,const std::string& webroot="web");
			webserver_t(const webserver_t& copy)=delete;
			~webserver_t();
			webserver_t& operator=(const webserver_t& copy)=delete;
			bool good() const;
			void open();
			void close();
			std::string address() const;
			std::string webroot() const;

		private:
			static int client_func_handler(mg_connection* connection,enum mg_event event);
			mg_server* server_m[10];
			client_func_t client_func_m;
			std::string address_m;
			std::string webroot_m;
	};
}

#endif