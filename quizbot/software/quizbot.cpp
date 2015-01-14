#include <iostream>
#include <mongoose/mongoose.h>
#include <msl/serial.hpp>
#include <msl/string.hpp>
#include <msl/time.hpp>
#include <mutex>
#include <string>
#include <thread>

std::string port="/dev/ttyUSB0";
size_t baud=57600;
msl::serial arduino(port,baud);

std::string question="What is 9*8?";
std::string answer_a="72";
std::string answer_b="81";
std::string answer_c="17";

void send_data(mg_connection* connection,const std::string& type,const std::string& str)
{
	mg_printf
	(
		connection,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"%s",
		type.c_str(),str.size(),str.c_str()
	);
}

int client_func(mg_connection* connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(connection==nullptr||event!=MG_REQUEST)
		return false;

	std::string request(connection->uri);

	std::cout<<"Connection received from "<<connection->remote_ip<<":"<<connection->remote_port<<
		" requesting \""<<request<<"\"."<<std::endl;

	if(request=="/status")
	{
		if(arduino.good())
			send_data(connection,"text/json","{\"connected\":true}");
		else
			send_data(connection,"text/json","{\"connected\":false}");

		return true;
	}
	else if(request=="/question")
	{
		std::string json="{";
		json+="\"question\":\""+question+"\",";
		json+="\"answer_a\":\""+answer_a+"\",";
		json+="\"answer_b\":\""+answer_b+"\",";
		json+="\"answer_c\":\""+answer_c+"\"}";
		send_data(connection,"text/json",json);
		return true;
	}
	else if(request.size()==6&&msl::starts_with(request,"/ans="))
	{
		bool correct=false;

		if(request=="/ans=A")
			correct=true;
		else if(request!="/ans=B"&&request!="/ans=C")
			return false;

		if(correct)
		{
			send_data(connection,"text/json","{\"correct\":true}");
			arduino.write("y");
		}
		else
		{
			send_data(connection,"text/json","{\"correct\":false}");
			arduino.write("n");
		}

		return true;
	}

	return false;
}

void arduino_thread_func()
{
	while(true)
	{
		arduino.open();

		if(arduino.good())
		{
			std::cout<<"Arduino found on "<<port<<"@"<<baud<<"."<<std::endl;

			while(arduino.good())
			{}
		}

		arduino.close();
		//std::cout<<"Arduino not found on "<<port<<"@"<<baud<<"."<<std::endl;
		msl::delay_ms(500);
	}
}

int main()
{
	std::thread arduino_thread(arduino_thread_func);
	arduino_thread.detach();
	std::cout<<"Spawned arduino thread."<<std::endl;

	auto server=mg_create_server(nullptr,client_func);
	mg_set_option(server,"listening_port","8080");
	mg_set_option(server,"document_root","web");
	std::cout<<"Web server started."<<std::endl;

	while(true)
	{
		mg_poll_server(server,1000);
		msl::delay_ms(1);
	}

	return 0;
}