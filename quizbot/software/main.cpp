#include "arduino.hpp"
#include "juniorstar.hpp"

arduino_t arduino("/dev/ttyUSB0",57600);

bool client_func(juniorstar_client_t client)
{
	if(client.request["status"]=="yes")
	{
		client.response["connected"]=arduino.good();
		client.reply();
		return true;
	}
	else if(client.request["quiz"]=="yes")
	{
		client.response["question"]="What is 9*8?";
		client.response["answer_a"]=9*8;
		client.response["answer_b"]=9*9;
		client.response["answer_c"]=9+8;
		client.reply();
		return true;
	}
	else if(client.request["answer"]=="A")
	{
		client.response["correct"]=true;
		client.reply();
		arduino.write("y");
		return true;
	}
	else
	{
		client.response["correct"]=false;
		client.reply();
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