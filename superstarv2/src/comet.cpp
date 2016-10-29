//Mike Moss
//09/24/2016
//Contains comet manager for long tail based requests.

#include "comet.hpp"

#include "auth.hpp"
#include "mongoose_util.hpp"
#include "string_util.hpp"
#include "time_util.hpp"

//Comet constants.
static const size_t max_comets=200;
static const int64_t comet_timeout_ms=1000*60*5;

//Handles connection conn with response as the template response.
//  Note, expects response to have:
//        .comet=true
//        .path=original_path_requested (in pathify form)
//        .result=original thing in path (this is how we know to update)
void comet_mgr_t::handle(mg_connection* conn,const Json::Value& response)
{
	//Mark connection that it's being taken care of.
	conn->flags|=MG_F_USER_1;

	//Make new client.
	comet_client_t client;
	client.conn=conn;
	client.expiration=millis()+comet_timeout_ms;
	client.response=response;

	//Room in queue, add client and return.
	if(clients_m.size()<max_comets)
	{
		clients_m.push_back(client);
		return;
	}

	//Queue full, kill oldest and add newest to the queue...
	cancel(clients_m[0]);
	clients_m.erase(clients_m.begin());
	clients_m.push_back(client);
}

//Goes through clients and cancels expired clients.
void comet_mgr_t::update()
{
	//Go through clients.
	std::vector<comet_client_t> remaining;
	for(size_t ii=0;ii<clients_m.size();++ii)
		//Experied, cancel.
		if(clients_m[ii].expiration<=millis())
			cancel(clients_m[ii]);

		//Still waiting...
		else
			remaining.push_back(clients_m[ii]);
	clients_m=remaining;
}

//Goes through clients and, if they are waiting on the path and
//  value is different, sends new value.
//  Note, also clears expired clients.
//  Note, path should be in pathify form.
void comet_mgr_t::update_path(const std::string& path,superstar_t& superstar)
{
	//Go through clients.
	std::vector<comet_client_t> remaining;
	for(size_t ii=0;ii<clients_m.size();++ii)
	{
		//Expired, cancel.
		if(clients_m[ii].expiration<=millis())
		{
			cancel(clients_m[ii]);
		}

		//Note expired, check for an update.
		else
		{
			//Get old/new value.
			std::string our_path(clients_m[ii].response["path"].asString());
			Json::Value& result=clients_m[ii].response["result"];
			std::string last_hash(result["hash"].asString());
			std::string old_value(JSON_serialize(result["value"]));
			Json::Value current_value(superstar.get(our_path));
			std::string new_hash(hash_sha256_hex(JSON_serialize(current_value)));

			//New value, update.
			if(last_hash!=new_hash||old_value!=JSON_serialize(current_value))
				service(clients_m[ii],current_value,new_hash);

			//Still waiting...
			else
				remaining.push_back(clients_m[ii]);
		}
	}
	clients_m=remaining;
}

//Cancels a connection and sends a response.
void comet_mgr_t::cancel(comet_client_t& client)
{
	//Send error and close socket.
	mg_send(client.conn,"408 Request Timeout","");
	client.conn->flags&=~MG_F_USER_1;
}

//Cancels a connection when mongoose kills it.
void comet_mgr_t::cancel(mg_connection* conn)
{
	//Go through clients.
	std::vector<comet_client_t> remaining;
	for(size_t ii=0;ii<clients_m.size();++ii)
		//Disconnecting, kill...
		if(clients_m[ii].conn==conn)
			cancel(clients_m[ii]);

		//Still waiting...
		else
			remaining.push_back(clients_m[ii]);
	clients_m=remaining;
}

//Services a connection with the given value.
void comet_mgr_t::service(comet_client_t& client,const Json::Value& value,
	const std::string& new_hash)
{
	//Clear unexpected members.
	client.response.removeMember("comet");
	client.response.removeMember("path");
	client.response["result"]["hash"]=new_hash;

	//Set response, send response, and close socket.
	client.response["result"]["value"]=value;
	mg_send(client.conn,"200 OK",JSON_serialize(client.response));
	client.conn->flags&=~MG_F_USER_1;
}