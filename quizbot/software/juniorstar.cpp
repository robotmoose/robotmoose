#include "juniorstar.hpp"

#include <chrono>
#include <iostream>
#include <thread>

juniorstar_client_t::juniorstar_client_t(const json::Object& json,mg_connection* connection):
	request(json),connection_m(connection)
{}

void juniorstar_client_t::reply(const json::Object& json) const
{
	if(connection_m!=nullptr)
		mg_printf
		(
			connection_m,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %ld\r\n"
			"\r\n"
			"%s",
			"text/json",
			json::Serialize(json).size(),json::Serialize(json).c_str()
		);
}

juniorstar_t::juniorstar_t(client_func_t client_func,const uint16_t port,const std::string webroot):
	server_m(nullptr),client_func_m(client_func),port_m(port),webroot_m(webroot)
{}

juniorstar_t::~juniorstar_t()
{
	stop();
}

bool juniorstar_t::good() const
{
	return server_m!=nullptr;
}

void juniorstar_t::start(const bool detach)
{
	stop();
	auto server=mg_create_server(this,juniorstar_t::client_func_handler);
	mg_set_option(server,"listening_port",std::to_string(port_m).c_str());
	mg_set_option(server,"document_root",webroot_m.c_str());

	if(server!=nullptr&&mg_poll_server(server,10))
		server_m=server;

	if(good())
	{
		std::thread server_thread(&juniorstar_t::server_thread_func_m,this);

		if(detach)
			server_thread.detach();
		else
			server_thread.join();
	}
}

void juniorstar_t::stop()
{
	if(server_m!=nullptr)
	{
		mg_destroy_server(&server_m);
		server_m=nullptr;
	}
}

uint16_t juniorstar_t::get_port() const
{
	return std::stoi(mg_get_option(server_m,"listening_port"));
}

void juniorstar_t::set_port(const uint16_t port,const bool restart)
{
	port_m=port;

	if(restart)
	{
		stop();
		start();
	}
}

std::string juniorstar_t::get_webroot() const
{
	return mg_get_option(server_m,"document_root");
}

void juniorstar_t::set_webroot(const std::string& webroot,const bool restart)
{
	webroot_m=webroot;

	if(restart)
	{
		stop();
		start();
	}
}

int juniorstar_t::client_func_handler(mg_connection* connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(connection==nullptr||event!=MG_REQUEST)
		return false;

	std::string request(connection->uri);

	std::cout<<"Connection received from "<<connection->remote_ip<<":"<<connection->remote_port<<
		" requesting \""<<request<<"\"."<<std::endl;

	if(request.size()>0)
	{
		auto json=json::Deserialize(request.substr(1,request.size()-1));

		if(json.GetType()==json::ObjectVal)
			return ((juniorstar_t*)(connection->server_param))->client_func_m({json,connection});
	}

	return false;
}

void juniorstar_t::server_thread_func_m()
{
	while(mg_poll_server(server_m,10))
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	stop();
}