//Mike Moss
//09/24/2016
//Contains comet manager for long tail based requests.

#ifndef COMET_HPP
#define COMET_HPP

#include <jsoncpp/json.h>
#include "json_util.hpp"
#include <mongoose/mongoose.h>
#include <string>
#include "superstar.hpp"
#include <vector>

//Client stucture used in comet_mgr_t...
struct comet_client_t
{
	mg_connection* conn;
	int64_t expiration;
	Json::Value response;
};

//Manager class (keeps track of clients, closes and cleans up connections)...
class comet_mgr_t
{
	public:
		//Handles connection conn with response as the template response.
		//  Note, expects response to have:
		//        .comet=true
		//        .path=original_path_requested (in pathify form)
		//        .result=original thing in path (this is how we know to update)
		void handle(mg_connection* conn,const Json::Value& response);


		//Goes through clients and cancels expired clients.
		void update();


		//Goes through clients and, if they are waiting on the path and
		//  value is different, sends new value.
		//  Note, also clears expired clients.
		//  Note, path should be in pathify form.
		void update_path(const std::string& path,superstar_t& superstar);


		//Cancels a connection and sends a response.
		void cancel(comet_client_t& client);

		//Cancels a connection when mongoose kills it.
		void cancel(mg_connection* conn);


		//Services a connection with the given value.
		void service(comet_client_t& client,const Json::Value& value,const std::string& last_hash);

	private:
		std::vector<comet_client_t> clients_m;
};

#endif