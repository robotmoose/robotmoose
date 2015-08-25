#include "ini.h"

#include <stdexcept>
#include <stdlib.h>

#include "ini.h"
#include "string_util.h"

robot_config_t::robot_config_t():name("test/demo"),superstar("http://robotmoose.com"),
	baudrate(57600),motors(""),marker(""),sensors(""),trim(1.0),debug(false),delay_ms(10),
	sim(false)
{}

void robot_config_t::from_file(const std::string& filename)
{
	ini_t ini=read_ini(filename);

	for(ini_t::const_iterator iter=ini.begin();iter!=ini.end();++iter)
	{
		if(iter->first=="robot")
			name=iter->second;
		else if(iter->first=="superstar")
			superstar=iter->second;
		else if(iter->first=="baudrate")
			baudrate=atoi(iter->second.c_str());
		else if(iter->first=="motors")
			motors+=iter->second+"\n";
		else if(iter->first=="marker")
			marker=iter->second;
		else if(iter->first=="sensors")
			sensors+=iter->second+"\n";
		else if(iter->first=="trim")
			trim=atof(iter->second.c_str());
		else if(iter->first=="debug")
		{
			debug=true;

			if(iter->second=="false"||iter->second=="0"||iter->second=="")
				debug=false;
		}
		else if(iter->first=="delay_ms")
			delay_ms=atoi(iter->second.c_str());
		else if(iter->first=="sim")
		{
			sim=true;

			if(iter->second=="false"||iter->second=="0"||iter->second=="")
				sim=false;

			if(sim)
			{
				delay_ms=100;
				baudrate=0;
			}
		}
		else
			throw std::runtime_error("Unrecognized ini argument \""+iter->first+"\".\n");
	}

	for(size_t ii=0;ii<name.size();++ii)
		if(name[ii]=='\\')
			name[ii]='/';

	if(superstar.size()>0&&superstar[superstar.size()-1]!='/')
			superstar+='/';
}

void robot_config_t::from_cli(int argc,char* argv[])
{
	for(int argi = 1; argi<argc; argi++)
	{
		if      (0 == strcmp(argv[argi], "--robot"))      {name = argv[++argi];}
		else if (0 == strcmp(argv[argi], "--superstar"))  superstar = argv[++argi];
		else if (0 == strcmp(argv[argi], "--local"))      superstar = "http://localhost:8081";
		else if (0 == strcmp(argv[argi], "--baudrate"))   baudrate = atoi(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--motor"))      motors += argv[++argi]+std::string("\n");
		else if (0 == strcmp(argv[argi], "--marker"))     marker = argv[++argi];
		else if (0 == strcmp(argv[argi], "--sensor"))     sensors += argv[++argi]+std::string("\n");
		else if (0 == strcmp(argv[argi], "--trim"))       trim = atof(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--debug"))      debug = true;
		else if (0 == strcmp(argv[argi], "--dev"))        superstar = "http://test.robotmoose.com" ;
		else if (0 == strcmp(argv[argi], "--delay_ms"))   delay_ms = atoi(argv[++argi]);
		else if (0 == strcmp(argv[argi], "--sim"))        // no Arduino, for debugging
		{
			sim=true;
			delay_ms=100;
			baudrate=0;
		}
		else
			throw std::runtime_error("Unrecognized command line argument \""+std::string(argv[argi])+"\".\n");
	}

	for(size_t ii=0;ii<name.size();++ii)
		if(name[ii]=='\\')
			name[ii]='/';

	if(superstar.size()>0&&superstar[superstar.size()-1]!='/')
		superstar+='/';
}

void robot_config_t::to_file(const std::string& filename) const
{
	ini_t ini;
	ini["robot"]=to_lower(name);
	ini["superstar"]=to_lower(superstar);
	ini["baudrate"]=to_lower(to_string(baudrate));
	ini["motors"]=to_lower(motors);
	ini["marker"]=to_lower(marker);
	ini["sensors"]=to_lower(sensors);
	ini["trim"]=to_lower(to_string(trim));
	ini["debug"]=to_lower(to_string(debug));
	ini["delay_ms"]=to_lower(to_string(delay_ms));
	ini["sim"]=to_lower(to_string(sim));
	write_ini(filename,ini);
}

bool robot_config_t::is_valid() const
{
	size_t slashes=0;

	for(size_t ii=0;ii<name.size();++ii)
		if(name[ii]=='/')
			++slashes;

	if(slashes!=1)
		throw std::runtime_error("Invalid robot name \""+name+"\" - should be in the format \"school/name\".");

	return false;
}