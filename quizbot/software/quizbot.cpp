#include <iostream>
#include <mongoose/mongoose.h>
#include <msl/serial.hpp>
#include <msl/string.hpp>
#include <msl/time.hpp>
#include <mutex>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <thread>

std::string port="/dev/ttyUSB0";
size_t baud=57600;
msl::serial arduino(port,baud);
rapidjson::Document quiz;

std::string stringify(const rapidjson::Document& json);
void send_data(mg_connection* connection,const std::string& type,const std::string& str);
int client_func(mg_connection* connection,enum mg_event event);
void arduino_thread_func();

int main()
{
	quiz.SetObject();
	quiz.AddMember("question","What is 9*8?",quiz.GetAllocator());
	quiz.AddMember("answer_a",72,quiz.GetAllocator());
	quiz.AddMember("answer_b",81,quiz.GetAllocator());
	quiz.AddMember("answer_c",17,quiz.GetAllocator());

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

std::string stringify(const rapidjson::Document& json)
{
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	json.Accept(writer);
	return strbuf.GetString();
}

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
		rapidjson::Document status;
		status.SetObject();
		status.AddMember("connected",arduino.good(),status.GetAllocator());
		send_data(connection,"text/json",stringify(status));
		return true;
	}
	else if(request=="/quiz")
	{
		send_data(connection,"text/json",stringify(quiz));
		return true;
	}
	else if(request.size()==6&&msl::starts_with(request,"/ans="))
	{
		bool correct=false;

		if(request=="/ans=A")
			correct=true;
		else if(request!="/ans=B"&&request!="/ans=C")
			return false;

		rapidjson::Document reply;
		reply.SetObject();
		reply.AddMember("correct",correct,reply.GetAllocator());
		send_data(connection,"text/json",stringify(reply));

		if(correct)
			arduino.write("y");
		else
			arduino.write("n");

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
			{msl::delay_ms(1);}
		}

		arduino.close();
		std::cout<<"Arduino not found on "<<port<<"@"<<baud<<"."<<std::endl;
		msl::delay_ms(500);
	}
}