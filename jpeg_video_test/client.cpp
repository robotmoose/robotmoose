//needs -luriparser

#include <iostream>
#include <msl/time.hpp>
#include <msl/webserver.hpp>
#include "webcam.hpp"
#include <fstream>
#include <uriparser/Uri.h>
#include <stdexcept>

void send_jpg(const mg_connection& connection,const std::string& jpg);

bool client_func(const mg_connection& connection,enum mg_event event);

msl::webserver_t server(client_func,"0.0.0.0:8081","web");
webcam_t cam;

int main()
{
	server.open();

	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(server.good())
	{
		size_t num=0;
		cam.open(num);

		if(cam.good())
			std::cout<<"Connected with camera "<<num<<"."<<std::endl;

		while(cam.good())
			msl::delay_ms(1);

		std::cout<<"Lost connection with camera "<<num<<"."<<std::endl;
		msl::delay_ms(1);
	}

	server.close();

	return 0;
}

void send_jpg(const mg_connection& connection,const std::string& jpg)
{
	std::string data;
	data+=jpg;
	mg_send_data((mg_connection*)&connection,data.data(),data.size());
}

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	std::cout<<connection.uri<<std::endl;

	if(std::string(connection.uri)=="/cam.jpg")
	{
		size_t quality=30;

		if(connection.query_string!=nullptr)
		{
			std::string request="?"+std::string(connection.query_string);

			UriParserStateA state;
			UriUriA uri;
			state.uri=&uri;

			if(uriParseUriA(&state,request.c_str())!=0)
			{
				std::cout<<"failed to parse \""<<request<<"\"!"<<std::endl;
				uriFreeUriMembersA(&uri);
				return false;
			}

			UriQueryListA* queries;

			if(uriDissectQueryMallocA(&queries,nullptr,uri.query.first,uri.query.afterLast)!=0)
			{
				std::cout<<"failed to decode \""<<request<<"\"!"<<std::endl;
				uriFreeUriMembersA(&uri);
				return false;
			}

			auto head=queries;

			while(head!=nullptr&&head->key!=nullptr&&head->value!=nullptr)
			{
				std::string key=head->key;
				std::string value=head->value;

				if(key=="quality")
				{
					try{quality=std::stoi(value);}
					catch(...){}
				}

				head=head->next;
			}

			uriFreeQueryListA(queries);
			uriFreeUriMembersA(&uri);
		}

		send_jpg(connection,cam.jpg(quality));
		return true;
	}

	return false;
}
