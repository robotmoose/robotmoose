#ifndef MSL_C11_WEBSERVER_HPP
#define MSL_C11_WEBSERVER_HPP

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "mongoose/mongoose.h"

namespace msl
{
	void client_reply(const mg_connection& client,const std::string& data,const std::string& mime);

	class webserver_t
	{
		public:
			class server_thread_t
			{
				public:
					server_thread_t(mg_server* server_ptr);

					mg_server* server;
					bool stop;
					std::mutex mutex;
			};

			typedef std::function<bool(const mg_connection& connection,mg_event event)> client_func_t;

			webserver_t(client_func_t client_func,const std::string& address,
				const std::string& webroot="web",const size_t thread_count=16);
			webserver_t(const webserver_t& copy)=delete;
			~webserver_t();
			webserver_t& operator=(const webserver_t& copy)=delete;

			void open();
			void close();

			bool good() const;
			std::string address() const;
			std::string webroot() const;
			size_t thread_count() const;

		private:
			client_func_t client_func_m;
			std::vector<server_thread_t*> threads_m;
			std::string address_m;
			std::string webroot_m;
			const size_t thread_count_m;

		friend int msl::client_thread_func_m(mg_connection* connection,mg_event event);
		friend void msl::server_thread_func_m(webserver_t::server_thread_t* thread);
	};

	int client_thread_func_m(mg_connection* connection,mg_event event);
	void server_thread_func_m(webserver_t::server_thread_t* thread);
}

#endif