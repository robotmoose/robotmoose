#include "webserver.hpp"

#include <chrono>
#include <thread>

int msl::client_thread_func_m(mg_connection* connection,mg_event event)
{
	if(connection!=nullptr)
	{
		if(((msl::webserver_t*)(connection->server_param))->client_func_m(*connection,event))
			return MG_TRUE;
		else
			return MG_FALSE;
	}

	return MG_FALSE;
}

static void server_thread_func_m(mg_server* server)
{
	while(mg_poll_server(server,10))
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void msl::client_reply(const mg_connection& client,const std::string& data,const std::string& mime)
{
	mg_printf
	(
		(mg_connection*)&client,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		mime.c_str(),
		data.size(),data.c_str()
	);
}

msl::webserver_t::webserver_t(client_func_t client_func,const std::string& address,
	const std::string& webroot,const size_t thread_count):
	client_func_m(client_func),address_m(address),webroot_m(webroot),thread_count_m(thread_count)
{
	if(thread_count_m<1)
		thread_count_m=1;
}

msl::webserver_t::~webserver_t()
{
	close();
}

bool msl::webserver_t::good() const
{
	//for(size_t ii=0;ii<threads_m.size();++ii)
	//	if(threads_m[ii]==nullptr||!mg_poll_server(threads_m[ii],10))
	//		return false;

	return (threads_m.size()==thread_count_m);
}

void msl::webserver_t::open()
{
	for(size_t ii=0;ii<thread_count_m;++ii)
	{
		threads_m.push_back(mg_create_server(this,msl::client_thread_func_m));

		if(threads_m[ii]==nullptr||!mg_poll_server(threads_m[ii],10))
		{
			close();
			return;
		}
	}

	if(threads_m.size()>0)
	{
		if(mg_set_option(threads_m[0],"listening_port",address_m.c_str())!=0)
		{
			close();
			return;
		}
	}

	for(size_t ii=1;ii<threads_m.size();++ii)
		mg_copy_listeners(threads_m[0],threads_m[ii]);

	for(size_t ii=0;ii<threads_m.size();++ii)
	{
		if(mg_set_option(threads_m[ii],"document_root",webroot_m.c_str())!=0)
		{
			close();
			return;
		}

		std::thread thread(server_thread_func_m,threads_m[ii]);
		thread.detach();
	}
}

void msl::webserver_t::close()
{
	for(size_t ii=0;ii<thread_count_m;++ii)
		mg_destroy_server(&threads_m[ii]);

	threads_m.clear();
}

std::string msl::webserver_t::address() const
{
	return address_m;
}

std::string msl::webserver_t::webroot() const
{
	return webroot_m;
}

size_t msl::webserver_t::thread_count() const
{
	return thread_count_m;
}
