#include "ini.h"

#include <stdexcept>
#include <stdlib.h>

#include "ini.h"
#include "string_util.h"

robot_config_t::robot_config_t()
{
	keys_m["robot"]="test/demo";                 // superstar robot name
	keys_m["superstar"]="http://robotmoose.com"; // superstar server
	keys_m["baudrate"]="57600";                  // serial comms to Arduino
	keys_m["motors"]="";                         // Arduino firmware device name
	keys_m["marker"]="";                         // computer vision marker file
	keys_m["sensors"]="";                        // All our sensors
	keys_m["trim"]="1.0";                        // proportional relation between left and right "motor" output
	keys_m["debug"]="false";                     // more output, more mess, but more data
	keys_m["delay_ms"]="10";                     // milliseconds to wait in control loop (be kind to CPU, network)
	keys_m["sim"]="false";                       // use without an arduino (for testing)
	keys_m["local"]="false";                     // use local server shortcut
	keys_m["dev"]="false";                       // use dev server shortcut

	validate();
}

void robot_config_t::from_file(const std::string& filename)
{
	ini_t ini=read_ini(filename);

	for(ini_t::const_iterator iter=ini.begin();iter!=ini.end();++iter)
	{
		if(keys_m.count(to_lower(iter->first))>0)
			keys_m[to_lower(iter->first)]=to_lower(iter->second);
		else
			throw std::runtime_error("Unrecognized ini argument \""+iter->first+"\".\n");
	}

	validate();
}

void robot_config_t::from_cli(int argc,char* argv[])
{
	for(int ii=1;ii<argc;++ii)
	{
		std::string original_key=argv[ii];
		std::string key=original_key;

		if(key.size()>1&&key[0]=='-'&&key[1]=='-')
		{
			key=key.substr(2,key.size()-2);
			std::string value="true";

			if(ii+1<argc)
			{
				std::string temp=argv[ii+1];

				if(!(temp.size()>1&&temp[0]=='-'&&temp[1]=='-'))
				{
					value=temp;
					++ii;
				}
			}

			if(keys_m.count(key)>0)
				keys_m[to_lower(key)]=to_lower(value);
			else
				throw std::runtime_error("Unrecognized cli argument \""+original_key+"\".\n");
		}
		else
		{
			throw std::runtime_error("Unrecognized cli argument \""+std::string(argv[ii])+"\".\n");
		}
	}

	validate();
}

void robot_config_t::to_file(const std::string& filename) const
{
	ini_t ini;

	for(key_t::const_iterator iter=keys_m.begin();iter!=keys_m.end();++iter)
		ini[iter->first]=to_lower(iter->second);

	write_ini(filename,ini);
}
std::string robot_config_t::get(const std::string& key)
{
	if(keys_m.count(key)>0)
		return keys_m[key];

	return "";
}

void robot_config_t::validate()
{
	for(key_t::const_iterator iter=keys_m.begin();iter!=keys_m.end();++iter)
		keys_m[iter->first]=to_lower(iter->second);

	for(size_t ii=0;ii<keys_m["robot"].size();++ii)
		if(keys_m["robot"][ii]=='\\')
			keys_m["robot"][ii]='/';

	if(to_bool(keys_m["sim"]))
	{
		keys_m["delay_ms"]="100";
		keys_m["baudrate"]="0";
	}

	if(to_bool(keys_m["local"]))
		keys_m["superstar"]="http://localhost:8081";

	if(to_bool(keys_m["dev"]))
		keys_m["superstar"]="http://test.robotmoose.com";

	if(keys_m["superstar"].size()>0&&keys_m["superstar"][keys_m["superstar"].size()-1]!='/')
		keys_m["superstar"]+='/';

	size_t slashes=0;

	for(size_t ii=0;ii<keys_m["robot"].size();++ii)
		if(keys_m["robot"][ii]=='/')
			++slashes;

	if(slashes!=1)
		throw std::runtime_error("Invalid robot name \""+keys_m["robot"]+"\" - should be in the format \"school/name\".");
}

/*void robot_config_t::print()
{
	for(key_t::const_iterator iter=keys_m.begin();iter!=keys_m.end();++iter)
		std::cout<<"\""<<iter->first<<"\":\""<<to_lower(iter->second)<<"\""<<std::endl;;
}*/