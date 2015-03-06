#include "webserver.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>
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

void msl::server_thread_func_m(msl::webserver_t::server_thread_t* thread)
{
	while(mg_poll_server(thread->server,10))
	{
		if(thread->stop==true)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	mg_destroy_server(&thread->server);
	thread->stop=true;
	thread->mutex.unlock();
}

msl::webserver_t::server_thread_t::server_thread_t(mg_server* server_ptr):server(server_ptr),stop(false)
{}

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
	client_func_m(client_func),address_m(address),webroot_m(webroot),
	thread_count_m(std::max((size_t)1,thread_count))
{}

msl::webserver_t::~webserver_t()
{
	close();
}

void msl::webserver_t::open()
{
	for(size_t ii=0;ii<thread_count_m;++ii)
	{
		threads_m.push_back(new server_thread_t(mg_create_server(this,msl::client_thread_func_m)));

		if(threads_m[ii]->server==nullptr||!mg_poll_server(threads_m[ii]->server,10))
		{
			close();
			return;
		}
	}

	if(threads_m.size()>0)
	{
		if(mg_set_option(threads_m[0]->server,"listening_port",address_m.c_str())!=0)
		{
			close();
			return;
		}
	}

	for(size_t ii=1;ii<threads_m.size();++ii)
		mg_copy_listeners(threads_m[0]->server,threads_m[ii]->server);

	for(size_t ii=0;ii<threads_m.size();++ii)
	{
		if(mg_set_option(threads_m[ii]->server,"document_root",webroot_m.c_str())!=0)
		{
			close();
			return;
		}
	}

	for(size_t ii=0;ii<threads_m.size();++ii)
	{
		threads_m[ii]->mutex.lock();
		std::thread thread(server_thread_func_m,threads_m[ii]);
		thread.detach();
	}
}

void msl::webserver_t::close()
{
	for(size_t ii=0;ii<threads_m.size();++ii)
	{
		if(!threads_m[ii]->stop)
		{
			threads_m[ii]->stop=true;
			threads_m[ii]->mutex.lock();
			threads_m[ii]->mutex.unlock();
		}

		delete threads_m[ii];
	}

	threads_m.clear();
}

bool msl::webserver_t::good() const
{
	return (threads_m.size()==thread_count_m);
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