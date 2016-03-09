#include <iostream>
#include <map>
#include <mutex>
#include <msl/time.hpp>
#include "uri.hpp"
#include <msl/webserver.hpp>

bool client_func(const mg_connection& connection,enum mg_event event);

msl::webserver_t server(client_func,"0.0.0.0:80","web");

std::mutex cam_lock;
std::map<std::string,std::string> cams={{"172.20.227.148:8081",""}};

int main()
{
	server.open();

	if(server.good())
		std::cout<<":)"<<std::endl;
	else
		std::cout<<":("<<std::endl;

	while(server.good())
	{
		for(auto& ii:cams)
		{
			auto jpg=get_jpg(ii.first,"/cam.jpg");
			cam_lock.lock();
			ii.second=jpg;
			cam_lock.unlock();
		}
		msl::delay_ms(10);
	}

	server.close();

	return 0;
}

bool client_func(const mg_connection& connection,enum mg_event event)
{
	if(event==MG_AUTH)
		return true;
	if(event!=MG_REQUEST)
		return false;

	if(connection.uri!=nullptr)
	{
		std::string request=connection.uri;
		std::cout<<connection.remote_ip<<":"<<connection.remote_port<<"\t"<<request<<std::endl;

		if(request=="/cam.jpg")
		{
			std::string who="";
			std::string jpg="";

			if(connection.query_string!=nullptr)
				for(auto ii:uri_parse_query("?"+std::string(connection.query_string)))
					if(ii.first=="who")
						who=ii.second;

			if(cams.count(who)>0)
			{
				cam_lock.lock();
				jpg=cams[who];
				cam_lock.unlock();
			}

			if(jpg!="")
			{
				send_jpg(connection,jpg);
				return true;
			}

			return false;
		}
	}

	return false;
}
