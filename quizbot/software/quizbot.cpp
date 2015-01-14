#include <stdexcept>
#include <iostream>
#include <mongoose/mongoose.h>
#include <msl/serial.hpp>
#include <msl/string.hpp>
#include <msl/time.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <json.h>

//Globals
json::Object setup(json::Deserialize
(R"(
	{"port":"8080"},
	{"webroot":"web"},
	{"serial":"/dev/ttyUSB0"},
	{"baud":"57600"}
)"));
json::Object quiz(json::Deserialize
(R"(
	{"question":"What is 9*8?"},
	{"answer_a":"72"},
	{"answer_b":"81"},
	{"answer_c":"17"}
)"));
msl::serial arduino("",0);

//Function Declarations
json::Object args_to_json(const int argc,char* argv[]);
void send_data(mg_connection* connection,const std::string& type,const std::string& str);
int client_func(mg_connection* connection,enum mg_event event);
void arduino_thread_func();

int main(int argc,char* argv[])
{
	//Get Command Line Arguments
	auto args=args_to_json(argc,argv);

	for(auto ii:args)
	{
		if(ii.first=="help")
		{
			std::cout<<"\tusage: ./quizbot --port 8080 --webroot web --serial /dev/ttyUSB0 --baud 57600"<<std::endl;
			return 0;
		}

		if(!setup.HasKey(ii.first))
		{
			std::cout<<"\tunknown command line argument \""+ii.first+"\"."<<std::endl;
			std::cout<<"\tusage: ./quizbot --port 8080 --webroot web --serial /dev/ttyUSB0 --baud 57600"<<std::endl;
			return 1;
		}

		setup[ii.first]=ii.second;
	}

	//Create Server
	auto server=mg_create_server(nullptr,client_func);
	mg_set_option(server,"listening_port",std::string(setup["port"]).c_str());
	mg_set_option(server,"document_root",std::string(setup["webroot"]).c_str());

	if(!mg_poll_server(server,10))
	{
		std::cout<<"Web server failed to start on port "<<std::string(setup["port"])<<"."<<std::endl;
		return 1;
	}

	std::cout<<"Web server started on port "<<std::string(setup["port"])<<"."<<std::endl;
	std::cout<<"Web root is \""<<std::string(setup["webroot"])<<"\"."<<std::endl;

	//Create Arduino Thread
	arduino=msl::serial(setup["serial"],std::stoi(setup["baud"]));
	std::thread arduino_thread(arduino_thread_func);
	arduino_thread.detach();

	std::cout<<"Arduino thread started."<<std::endl;

	//Web Server Loop
	while(mg_poll_server(server,10))
		msl::delay_ms(1);

	std::cout<<"Web server terminated."<<std::endl;

	return 0;
}

json::Object args_to_json(const int argc,char* argv[])
{
	json::Object args;

	for(int ii=1;ii<argc;++ii)
	{
		std::string var(argv[ii]);

		if(var.size()>2&&msl::starts_with(var,"--"))
			var=var.substr(2,var.size()-2);
		else if(var.size()>1&&msl::starts_with(var,"-"))
			var=var.substr(1,var.size()-1);

		std::string val="";

		if(ii+1<argc)
		{
			val=std::string(argv[ii+1]);

			if(msl::starts_with(val,"--")||msl::starts_with(val,"-"))
				val="";
			else
				++ii;
		}

		args[var]=val;
	}

	return args;
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
		json::Object status;
		status["connected"]=arduino.good();
		send_data(connection,"text/json",json::Serialize(status));
		return true;
	}
	else if(request=="/quiz")
	{
		send_data(connection,"text/json",json::Serialize(quiz));
		return true;
	}
	else if(request.size()==6&&msl::starts_with(request,"/ans="))
	{
		bool correct=false;

		if(request=="/ans=A")
			correct=true;
		else if(request!="/ans=B"&&request!="/ans=C")
			return false;

		json::Object reply;
		reply["correct"]=correct;
		send_data(connection,"text/json",json::Serialize(reply));

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
			std::cout<<"Arduino found on "<<(std::string)setup["serial"]<<"@"<<(std::string)setup["baud"]<<"."<<std::endl;

			while(arduino.good())
				msl::delay_ms(1);
		}

		arduino.close();
		std::cout<<"Arduino not found on "<<(std::string)setup["serial"]<<"@"<<(std::string)setup["baud"]<<"."<<std::endl;
		msl::delay_ms(500);
	}
}