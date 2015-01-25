//needs -luriparser

#include <iostream>
#include <msl/socket.hpp>
#include <msl/time.hpp>
#include <msl/webserver.hpp>
#include <uriparser/Uri.h>
#include <stdexcept>

std::string get_jpg(const std::string& who,const size_t quality);

void send_jpg(const mg_connection& connection,const std::string& jpg);

bool client_func(const mg_connection& connection,enum mg_event event);

msl::webserver_t server(client_func,"0.0.0.0:80","web");

int main()
{
	server.open();

	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(server.good())
		msl::delay_ms(1);

	server.close();

	return 0;
}

std::string get_jpg(const std::string& who)
{
	try
	{
		msl::tcp_socket_t get("0.0.0.0:0>"+who);
		get.open();

		if(!get.good())
		{
			std::cout<<"bad connection!"<<std::endl;
			return "";
		}

		std::string request="GET /cam.jpg HTTP/1.1\r\n";
		request+="Connection: close\r\n";
		request+="\r\n";
		get.write(request);

		std::cout<<"requesting:\n"<<request<<std::endl;

		std::string jpg="";
		uint8_t temp;

		while(get.available()>=0&&get.read(&temp,1)==1)
			jpg+=temp;

		get.close();

		bool found=false;

		for(size_t ii=0;ii<jpg.size();++ii)
		{
			if(ii+3<jpg.size()&&jpg[ii]==(char)0xff&&jpg[ii+1]==(char)0xd8&&jpg[ii+2]==(char)0xff&&jpg[ii+3]==(char)0xe0)
			{
				found=true;
				jpg=jpg.substr(ii,jpg.size()-ii);
				break;
			}
		}

		if(!found)
			return "";

		return jpg;
	}
	catch(...)
	{}

	return "";
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
		std::string who="";

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

				if(key=="who")
					who=value;

				head=head->next;
			}

			uriFreeQueryListA(queries);
			uriFreeUriMembersA(&uri);
		}

		send_jpg(connection,get_jpg(who));
		return true;
	}

	return false;
}
