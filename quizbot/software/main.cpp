#include <arduino.hpp>
#include "littlestar.hpp"

arduino_t arduino("/dev/ttyACM0",57600);

bool client_func(littlestar_client_t client)
{
	if(client.request["type"]=="status")
	{
		client.response["connected"]=arduino.good();
		client.reply();
		return true;
	}
	else if(client.request["type"]=="quiz")
	{
		client.response["question"]="What is 9*8?";
		client.response["answer_a"]=9*8;
		client.response["answer_b"]=9*9;
		client.response["answer_c"]=9+8;
		client.reply();
		return true;
	}
	else if(client.request["type"]=="answer")
	{
		if(client.request["answer"]=="A")
		{
			client.response["correct"]=true;
			arduino.write("y");
		}
		else
		{
			client.response["correct"]=false;
			arduino.write("n");
		}

		client.reply();
		return true;
	}

	return false;
}

int main()
{
	arduino.start();
	littlestar_t server(client_func,8080,"web");
	server.start();
	return 0;
}
