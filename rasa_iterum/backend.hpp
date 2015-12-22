#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <iostream>
#include <json.h>
#include <serial.h>
#include <string>
#include <vector>

class backend_t
{
	public:
		backend_t()
		{}

		std::string set(const std::string& json_str)
		{
			std::string error_str="";

			try
			{
				json::Value json=json::Deserialize(json_str);

				std::string school=json["school"];
				std::string robot=json["robot"];
				std::string port=json["port"];

				if(port!="")
				{
					bool found=false;
					std::vector<std::string> port_list=ports();

					for(size_t ii=0;ii<port_list.size();++ii)
					{
						if(port==port_list[ii])
						{
							found=true;
							break;
						}
					}

					if(!found)
						throw std::runtime_error("Invalid serial port \""+port+"\".");

					if(serial_m.get_name()!=port)
					{
						serial_m.Open(port);
						serial_m.Set_baud(57600);
					}
				}
				else
				{
					serial_m.Close();
				}

				school_m=school;
				robot_m=robot;

				std::cout<<school<<"\t"<<robot<<"\t"<<port<<std::endl;
			}
			catch(std::exception& error)
			{
				error_str=error.what();
			}
			catch(...)
			{
				error_str="Unknown error occured.";
			}

			return error_str;
		}

		std::string robot() const
		{
			return robot_m;
		}

		std::string school() const
		{
			return school_m;
		}

		std::string status()
		{
			if(serial_m.Is_open())
				return "connected";
			else
				return "disconnected";
		}

		std::string port()
		{
			if(serial_m.Is_open())
				return serial_m.get_name();
			else
				return "";
		}

		std::vector<std::string> ports()
		{
			std::vector<std::string> port_list=serial_m.port_list();

			for(size_t ii=0;ii<port_list.size();++ii)
				if(port_list[ii].find("Bluetooth")!=std::string::npos||port_list[ii].find("ttyAMA")!=std::string::npos)
					port_list.erase(port_list.begin()+ii);

			return port_list;
		}

		void update()
		{
			char temp;

			if(serial_m.available()>0)
			{
				if(serial_m.Read(&temp,1)==1)
				{}
				else
				{
					serial_m.Close();
				}
			}
		}

	private:
		SerialPort serial_m;
		std::string school_m;
		std::string robot_m;
};

#endif