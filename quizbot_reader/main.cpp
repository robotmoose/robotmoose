#include <arduino.hpp>
#include <iostream>
#include <msl/time.hpp>
#include "reader.hpp"
#include <stdexcept>
#include <string>
#include <ctime>

arduino_t arduino("/dev/ttyUSB0",57600,false);

void send_answer(const bool answer)
{
	if(arduino.good())
	{
		if(answer)
			arduino.write("y");
		else
			arduino.write("n");
	}
	else
	{
		std::cout<<"Arduino not found on "<<arduino.get_serial()<<"@"<<arduino.get_baud()<<"."<<std::endl;
	}
}

int main(int argc,char* argv[])
{
	try
	{
		if(argc>1)
			arduino.set_serial(argv[1]);
		if(argc>2)
			arduino.set_baud(std::stoi(argv[2]));

		srand(time(nullptr));

		arduino.start();

		for(auto question:read_questions("questions.ini"))
		{
			while(true)
			{
				auto answer=ask_question(question);
				send_answer(answer);

				if(answer)
					break;
			}
		}

		std::cout<<"All done!"<<std::endl;
	}
	catch(std::exception& e)
	{
		std::cout<<e.what()<<std::endl;
		return 1;
	}

	return 0;
}