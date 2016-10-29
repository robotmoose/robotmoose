//Mike Moss
//07/21/2016
//Contains the actual superstar server (mainly the http server portion).

#include "comet.hpp"
#include <fstream>
#include <iostream>
#include "json_util.hpp"
#include "jsonrpc.hpp"
#include <mongoose/mongoose.h>
#include "mongoose_util.hpp"
#include <stdexcept>
#include <sstream>
#include <string>
#include "string_util.hpp"
#include "superstar.hpp"
#include "time_util.hpp"
#include "web_util.hpp"

//Server options...needs to be global/non-local.
mg_serve_http_opts server_options;

//"Database" and comet manager.
superstar_t superstar("auth","db.json");
comet_mgr_t comet_mgr;

//Post log.
std::string post_log_name("superstar.log");
std::ofstream post_log(post_log_name.c_str());

//UUID variables...
int64_t last_uuid=0;

//Time variables...
const int64_t backup_time=20000;
int64_t backup_old_time=millis();
const int64_t comet_time=1000;
int64_t comet_old_time=millis();

void http_handler(mg_connection* conn,int event,void* event_data)
{
	//Giant paranoia try/catch...
	try
	{
		if(event==MG_EV_CLOSE)
		{
			comet_mgr.cancel(conn);
			return;
		}

		//We really only care about HTTP requests.
		if(event==MG_EV_HTTP_REQUEST)
		{
			http_message* msg=(http_message*)event_data;
			if(msg==NULL)
				return;

			//Get client IP address (Apache messes this up...).
			char client_raw[200];
			mg_sock_to_str(conn->sock,client_raw,200,MG_SOCK_STRINGIFY_IP);
			std::string client(client_raw);

			//Extract the stuff we care about.
			std::string method(msg->method.p,msg->method.len);
			std::string request(msg->uri.p,msg->uri.len);
			std::string query(msg->query_string.p,msg->query_string.len);
			std::string post_data(msg->body.p,msg->body.len);

			//Print out everything.
			std::cout<<"Connection: "<<client<<" "<<method<<" "<<request;
			if(query.size()>0)
				std::cout<<"?"<<query;
			std::cout<<std::endl;

			//Starting "/superstar/"...
			bool starts_with_superstar=starts_with(strip_end(request,"/"),
				"/superstar");

			//Log Post into a file.
			//  One JSON object per line, containing:
			//  {
			//      "time":TIME_MS_SINCE_LINUX_EPOCH,
			//      "client":STR_IP_ADDRESS,
			//      "data":STR_URL_ENCODED_DATA
			//  }
			if(method=="POST")
			{
				//Starting "/superstar/" means a JSON-RPC request.
				if(starts_with_superstar)
				{
					Json::Value entry;
					entry["time"]=Json::Int64(millis());
					entry["client"]=client;
					entry["data"]=url_encode(post_data);
					post_log<<JSON_serialize(entry)<<std::endl;
					jsonrpc(superstar,comet_mgr,post_data,conn);
				}

				//Normal post (status 200 apparently if POST does nothing).
				else
				{
					mg_send(conn,"200 OK","");
					return;
				}
			}

			//Get requests...
			else if(method=="GET")
			{

				//Starting "/superstar/" means a database query or UUID.
				if(starts_with_superstar)
				{
					//Query for UUID
					char buffer[10];
					if(mg_get_http_var(&msg->query_string,"uuid",buffer,10)>0)
					{
						int64_t uuid=millis();
						while(uuid<=last_uuid)
							++uuid;
						last_uuid=uuid;
						std::ostringstream ostr;
						ostr<<uuid;
						mg_send(conn,"200 OK",ostr.str());
						return;
					}

					//Database Query
					request=request.substr(10,request.size()-10);
					mg_send(conn,"200 OK",JSON_serialize(superstar.get(request)));
				}

				//Otherwise it's a document...
				else
				{
					mg_serve_http(conn,msg,server_options);
				}
			}

			//Everything else is not allowed...
			else
			{
				mg_send(conn,"405 Method Not Allowed","");
			}
		}
	}
	catch(std::exception& error)
	{
		std::cout<<"Request error: "<<error.what()<<std::endl;
		mg_send(conn,"400 Bad Request","");
	}
	catch(...)
	{
		std::cout<<"Request error: Unknown exception."<<std::endl;
		mg_send(conn,"400 Bad Request","");
	}
}

int main(int argc,char* argv[])
{
	try
	{
		//Get address via command line args (if given).
		std::string address("0.0.0.0:8081");
		if(argc==2)
			address=std::string(argv[1]);
		else if(argc!=1)
			throw std::runtime_error("Invalid command line args (./superstar [[ADDRESS:]PORT]).");

			//Try to load database...
		if(superstar.load())
			std::cout<<"Loaded backup database."<<std::endl;
		else
			std::cout<<"No backup database found."<<std::endl;

		if(!post_log)
			throw std::runtime_error("Could not open log file \""+post_log_name+"\"!");

		//Server settings.
		server_options.document_root="../www";
		server_options.enable_directory_listing="no";
		server_options.ssi_pattern="**.html$";

		//Create server.
		mg_mgr manager;
		mg_mgr_init(&manager,NULL);
		mg_connection* server_conn=mg_bind(&manager,address.c_str(),http_handler);
		if(server_conn==NULL)
			throw std::runtime_error("Could not bind to "+address+".");
		mg_set_protocol_http_websocket(server_conn);

		//Serve...forever...
		std::cout<<"Superstar started on "<<address<<":"<<std::endl;
		while(true)
		{
			//Update web...
			mg_mgr_poll(&manager,500);
			int64_t new_time;

			//Update comet...
			new_time=millis();
			if((new_time-comet_old_time)>=comet_time)
			{
				comet_mgr.update();
				comet_old_time=new_time;
			}

			//Try to save database...
			new_time=millis();
			if((new_time-backup_old_time)>=backup_time)
			{
				if(superstar.save())
					std::cout<<"Saved backup database."<<std::endl;
				else
					std::cout<<"Could not save backup database."<<std::endl;
				backup_old_time=new_time;
			}

		}
		mg_mgr_free(&manager);
	}
	catch(std::exception& error)
	{
		std::cout<<"Error: "<<error.what()<<std::endl;
		return 1;
	}
	catch(...)
	{
		std::cout<<"Error: Unknown exception."<<std::endl;
		return 1;
	}

	return 0;
}