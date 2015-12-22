#include "backend.hpp"
#include "handler.hpp"
#include "http_util.hpp"
#include <iostream>
#include <mongoose/mongoose.h>
#include <string>

int main()
{
	backend_t backend;
	mg_server* server=mg_create_server(&backend,http_handler);

	if(server==NULL)
	{
		std::cout<<"Error creating server."<<std::endl;
		return 1;
	}

	if(mg_set_option(server,"listening_port","127.0.0.1:8888")!=0)
	{
		std::cout<<"Already started."<<std::endl;
		open_site("http://localhost:8888");
		return 1;
	}

	mg_set_option(server,"ssi_pattern","**.html$");
	mg_set_option(server,"document_root","./www");

	std::cout<<"Rasa Iterum Started"<<std::endl;
	open_site("http://localhost:8888");

	while(true)
	{
		mg_poll_server(server,1000);
		backend.update();
	}

	return 0;
}
