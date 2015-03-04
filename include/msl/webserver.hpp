#ifndef MSL_C11_WEBSERVER_HPP
#define MSL_C11_WEBSERVER_HPP

#include <functional>
#include <string>
#include <vector>

#include "mongoose/mongoose.h"

namespace msl
{
	int client_thread_func_m(mg_connection* connection,mg_event event);
	void client_reply(const mg_connection& client,const std::string& data,const std::string& mime);

	class webserver_t
	{
		friend int msl::client_thread_func_m(mg_connection* connection,mg_event event);

		public:
			typedef std::function<bool(const mg_connection& connection,mg_event event)> client_func_t;

			webserver_t(client_func_t client_func,const std::string& address,
				const std::string& webroot="web",const size_t thread_count=16);
			webserver_t(const webserver_t& copy)=delete;
			~webserver_t();
			webserver_t& operator=(const webserver_t& copy)=delete;
			bool good() const;
			void open();
			void close();
			std::string address() const;
			std::string webroot() const;
			size_t thread_count() const;

		private:
			client_func_t client_func_m;
			std::vector<mg_server*> threads_m;
			std::string address_m;
			std::string webroot_m;
			size_t thread_count_m;
	};
}

#endif