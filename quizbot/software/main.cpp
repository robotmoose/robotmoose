#include "arduino.hpp"
#include "juniorstar.hpp"

arduino_t arduino("/dev/ttyUSB0",57600);

bool client_func(juniorstar_client_t client)
{
	if(client.request["status"]=="yes")
	{
		json::Object response;
		response["connected"]=arduino.good();
		client.reply(response);
		return true;
	}
	else if(client.request["quiz"]=="yes")
	{
		json::Object response;
		response["question"]="What is 9*8?";
		response["answer_a"]=9*8;
		response["answer_b"]=9*9;
		response["answer_c"]=9+8;
		client.reply(response);
		return true;
	}
	else if(client.request["answer"]=="A")
	{
		json::Object response;
		response["correct"]=true;
		client.reply(response);
		arduino.write("y");
		return true;
	}
	else
	{
		json::Object response;
		response["correct"]=false;
		client.reply(response);
		arduino.write("n");
		return true;
	}

	return false;
}

int main()
{
	arduino.start();
	juniorstar_t server(client_func,8080,"web");
	server.start();
	return 0;
}