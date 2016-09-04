#include "wget.hpp"
#include <stdexcept>
#include "string_util.hpp"

struct wget_t
{
	std::string data;
	std::string error;
	bool done;
};

static void wget_ev_handler(mg_connection* connection,int ev,void* ev_data)
{
	wget_t& responder=*(wget_t*)(connection->mgr->user_data);

	if(ev==MG_EV_CONNECT)
	{
		int status=*(int*)ev_data;

		if(status!=0)
			responder.error=strerror(status);
	}
	else if(ev==MG_EV_HTTP_REPLY)
	{
		connection->flags|=MG_F_CLOSE_IMMEDIATELY;
		http_message* hm=(http_message*)ev_data;

		if(hm->resp_code!=200)
			responder.error="Connection error: "+to_string(hm->resp_code)+".";
		else
			responder.data=std::string(hm->body.p,hm->body.len);
	}
	else if(ev==MG_EV_CLOSE)
	{
		responder.done=true;
	}
}

std::string wget(const std::string& address,const std::string& post_data)
{
	wget_t responder;
	responder.data="";
	responder.error="";
	responder.done=false;

	mg_mgr mgr;
	mg_mgr_init(&mgr,&responder);
	mg_connection* nc=mg_connect_http(&mgr,wget_ev_handler,address.c_str(),NULL,post_data.c_str());

	if(nc==NULL)
	{
		responder.error="Could not open a socket!";
	}
	else
	{
		while(!responder.done)
			mg_mgr_poll(&mgr,1000);

		mg_mgr_free(&mgr);
	}

	if(responder.error.size()>0)
		throw std::runtime_error(responder.error);

	return responder.data;
}