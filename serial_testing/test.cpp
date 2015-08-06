#include "serial.hpp"
#include <iostream>
#include <string>

int main()
{
	while(true)
	{
		serial_list_t ports=serial_t::list();

		for(size_t ii=0;ii<ports.size();++ii)
		{
			serial_t port(ports[ii]);
			std::cout<<port.name()<<" -> Opening..."<<port.open(57600)<<std::endl;

			std::string test;

			while(port.good())
			{
				char temp;

				while(port.available()>0&&port.read(&temp,1)==1)
				{
					std::cout<<temp<<std::flush;
					test+=temp;
				}

				if(test.size()>0&&test[test.size()-1]=='\n')
				{
					if(test[0]=='H')
						port.write("hello",5);

					test="";
				}
			}

			std::cout<<"Disconnected"<<std::endl;
		}
	}

	return 0;
}