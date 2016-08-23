#include "wget.hpp"
#include <stdexcept>

struct wget_t
{
	std::string data;
	std::string error;
	bool done;
};

void wget_ev_handler(mg_connection* connection,int ev,void* ev_data)
{
	wget_t& responder=*(wget_t*)(connection->mgr->user_data);

	if(ev==MG_EV_CONNECT)
	{
		int status=*(int*)ev_data;

		if(status!=0)
		{
			responder.error=strerror(status);
			responder.done=true;
			return;
		}
	}
	else if(ev==MG_EV_HTTP_REPLY)
	{
		connection->flags|=MG_F_CLOSE_IMMEDIATELY;
		http_message* hm=(http_message*)ev_data;

		if(hm->resp_code!=200)
		{
			responder.error="Connection error: "+std::to_string(hm->resp_code)+".";
			responder.done=true;
			return;
		}

		responder.data=std::string(hm->body.p,hm->body.len);
		responder.done=true;
	}
	else
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
	mg_connection* nc=mg_connect_http(&mgr,wget_ev_handler,address.c_str(),nullptr,post_data.c_str());

	if(nc==nullptr)
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