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

std::string port="/dev/ttyUSB0";
size_t baud=57600;
msl::serial arduino(port,baud);
json::Object quiz;

json::Object args_to_json(const int argc,char* argv[]);
void send_data(mg_connection* connection,const std::string& type,const std::string& str);
int client_func(mg_connection* connection,enum mg_event event);
void arduino_thread_func();

int main(int argc,char* argv[])
{
	auto args=args_to_json(argc,argv);

	quiz["question"]="What is 9*8?";
	quiz["answer_a"]=9*8;
	quiz["answer_b"]=9*9;
	quiz["answer_c"]=9+8;

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

json::Object args_to_json(const int argc,char* argv[])
{
	json::Object args;

	/*for(int ii=1;ii<argc;++ii)
	{
		std::string var(argv[ii]);
		bool valid=false;

		if(var.size()>2&&msl::starts_with(var,"--"))
		{
			valid=true;
			var=var.substr(2,var.size()-2);
		}
		else if(var.size()>1&&msl::starts_with(var,"-"))
		{
			valid=true;
			var=var.substr(1,var.size()-1);
		}

		if(!valid)
			throw std::runtime_error("args_to_json invalid argument \""+var+"\".");

		std::string val="";

		if(ii+1<argc)
		{
			val=std::string(argv[ii]);

			if(msl::starts_with(val,"--")||msl::starts_with(val,"-"))
				val="";
		}

		//if(val.size()>0)
			//args.AddMember(var,val,args.GetAllocator());
		//else
			//args.AddMember(var,true,args.GetAllocator());
	}*/

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
			std::cout<<"Arduino found on "<<port<<"@"<<baud<<"."<<std::endl;

			while(arduino.good())
			{msl::delay_ms(1);}
		}

		arduino.close();
		std::cout<<"Arduino not found on "<<port<<"@"<<baud<<"."<<std::endl;
		msl::delay_ms(500);
	}
}