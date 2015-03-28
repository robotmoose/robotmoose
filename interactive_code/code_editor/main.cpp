//Requires linux system with avr-g++ and avr-objcopy in bin search path.

#include "cppcheck.hpp"
#include <cstdlib>
#include <iostream>
#include <json.h>
#include <msl/time.hpp>
#include <msl/webserver.hpp>

bool client_func(const mg_connection& connection,enum mg_event event);
void send_string(const mg_connection& connection,const std::string& str);
void send_json(const mg_connection& connection,const json::Object& obj);
void code_check(const std::string& code,json::Object& response);

msl::webserver_t server(client_func,"0.0.0.0:8082","../../www",8);

int main()
{
	server.open();

	if(!server.good())
	{
		std::cout<<":("<<std::endl;
		return 0;
	}

	std::cout<<":)"<<std::endl;

	while(server.good())
		msl::delay_ms(10);

	return 0;
}

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	std::string method=connection.request_method;
	std::string request=connection.uri;
	std::cout<<connection.remote_ip<<":"<<connection.remote_port<<"\t"<<request<<std::endl;

	if(method=="POST"&& (request=="/code" || request=="/editor/code"))
	{
		std::string code=std::string(connection.content,connection.content_len);
		json::Object response;
		code_check(code,response);
		send_json(connection,response);
		return true;
	}

	return false;
}

void send_string(const mg_connection& connection,const std::string& str)
{
	mg_printf
	(
		(mg_connection*)&connection,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		"text/plain",
		str.size(),str.c_str()
	);
}

void send_json(const mg_connection& connection,const json::Object& obj)
{
	std::string str=json::Serialize(obj);

	mg_printf
	(
		(mg_connection*)&connection,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		"text/json",
		str.size(),str.c_str()
	);
}

void code_check(const std::string& code,json::Object& response)
{
	json::Array json_errors;
	std::vector<myerror_t> errors;

	if(cppcheck_arduino_anonymous(code,errors,100000))
	{
		for(auto error:errors)
		{
			json::Object json_error;
			json_error["line"]=(int)error.line;
			json_error["column"]=(int)error.column;
			json_error["text"]=error.text;
			json_errors.push_back(json_error);
		}
	}

	response["errors"]=json_errors;
}

