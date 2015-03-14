/*
  Simple configuration file parsing: this is how you get values into
  your simulation.

Dr. Orion Sky Lawlor, olawlor@gmail.com, 2011-01-25 (Public Domain)
*/
#ifndef __PHYSICS_CONFIG_H
#define __PHYSICS_CONFIG_H

#include <map>
#include <string>

typedef std::map<std::string /*keyword*/,std::string /*value*/> physics_cfg_keys; /* config file section: keyword/value pairs */
typedef std::map<std::string /*section*/,physics_cfg_keys> physics_cfg_file; /* entire config file: a list of named sections */

class physics_cfg {
public:
	static physics_cfg_file file; /* last-read config file */
	static void read(const char *filename="config.ini"); /* re-read this config file */

/* Look up a config file value of various types: 
	float, int, bool, or string.
  In config.ini, the value you're looking for will look like:

[section]
keyword = value; comment

The default_value is returned if there is no config file, or the file
doesn't contain your keyword.
*/
	static double value(const char *section,const char *keyword,
			double default_value,const char *comment="",
			double min_value=-1.0e30f,double max_value=+1.0e30f);
	
	static int value(const char *section,const char *keyword,
			int default_value,const char *comment="",
			int min_value=-999999999,int max_value=999999999);
	
	static bool value(const char *section,const char *keyword,
			bool default_value,const char *comment="");
	
	static std::string value(const char *section,const char *keyword,
			const char *default_value,const char *comment="");
};

#endif
