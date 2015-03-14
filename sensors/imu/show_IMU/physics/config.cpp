/*
  Simple configuration file parsing: this is how you get values into
  your simulation.

Dr. Orion Sky Lawlor, olawlor@gmail.com, 2011-01-24 (Public Domain)
*/
#include "config.h"
#include <fstream>
#include <cctype> /* for std::isspace */
#include <cstdlib> /* for std::atof */

/* Remove whitespace from start and end of this string */
static std::string squeeze(const std::string &src) {
	int i=0, j=src.size()-1; /* first and last real char */
	while (i<(int)src.size() && std::isspace(src[i])) 
		i++;
	while (j>=0 && std::isspace(src[j])) 
		j--;
	if (j<i) return ""; // all-white line
	else return src.substr(i,j+1-i);
}

physics_cfg_file physics_cfg_file_read(const std::string &name) {
	std::ifstream f(name.c_str());
	std::string line;
	std::string section="default";
	physics_cfg_file file;
	physics_cfg_keys *keys=0;
	while (std::getline(f,line)) {
		size_t comment=line.find(";"); // trim off comments
		if (comment<std::string::npos) line.erase(comment);

		// skip leading whitespace
		line=squeeze(line);

		if (line.size()==0) { /* blank line */ }
		else if (line[0]=='[') { // new section
			keys=0;

			size_t close=line.find("]");
			if (close==std::string::npos) continue; /* no closing brace */
			section=squeeze(line.substr(1,close-1));
		}
		else { // new keyword = value line
			if (keys==0) keys=&file[section];

			size_t equal=line.find("=");
			if (equal==std::string::npos) continue; /* no equals... */
			std::string keyword=squeeze(line.substr(0,equal));
			std::string value=squeeze(line.substr(equal+1));
			(*keys)[keyword]=value;
		}
	}
	return file;
}


physics_cfg_file physics_cfg::file; /* last-read config file */
void physics_cfg::read(const char *filename) /* re-read this config file */
{
	try {
		file=physics_cfg_file_read(filename);
	} catch (...) {
		/* ignore errors during read */
	}
}


/* Look up one config file value.  Returns a C string if found,
or NULL if not in the file. */
static const char *physics_cfg_lookup(const char *section,const char *keyword)
{
	physics_cfg_keys &keys=physics_cfg::file[section];
	physics_cfg_keys::iterator it=keys.find(keyword);
	if (it==keys.end()) return NULL;
	else return (*it).second.c_str();
}


/* Look up a config file value of various types: 
	float, int, bool, or string.
  In config.ini, the value you're looking for will look like:

[section]
keyword = value; comment

The default_value is returned if there is no config file, or the file
doesn't contain your keyword.
*/
double physics_cfg::value(const char *section,const char *keyword,
			double default_value,const char *comment,
			double min_value,double max_value)
{
	const char *v=physics_cfg_lookup(section,keyword);
	if (v==NULL) return default_value;
	else {
		double ret=default_value;
		sscanf(v,"%lg",&ret);
		if (ret<min_value) ret=min_value;
		if (ret>max_value) ret=max_value;
		return ret;
	}
}
	
int physics_cfg::value(const char *section,const char *keyword,
			int default_value,const char *comment,
			int min_value,int max_value)
{
	const char *v=physics_cfg_lookup(section,keyword);
	if (v==NULL) return default_value;
	else {
		int ret=default_value;
		sscanf(v,"%i",&ret);
		if (ret<min_value) ret=min_value;
		if (ret>max_value) ret=max_value;
		return ret;
	}
}
	
bool physics_cfg::value(const char *section,const char *keyword,
			bool default_value,const char *comment)
{
	const char *v=physics_cfg_lookup(section,keyword);
	if (v==NULL) return default_value;
	else {
		bool ret=default_value;
		char c=std::tolower(v[0]);
		if (c=='f' || c=='n' || c=='0') ret=false;
		if (c=='t' || c=='y' || c=='1') ret=true;
		return ret;
	}
}
	
std::string physics_cfg::value(const char *section,const char *keyword,
			const char *default_value,const char *comment)
{
	const char *v=physics_cfg_lookup(section,keyword);
	if (v==NULL) return default_value;
	else return v;
}



/********************* Testing Code ***************************/
#if 0
void print(const physics_cfg_file &file) {
	for (physics_cfg_file::const_iterator it=file.begin();it!=file.end();++it) {
		std::cout<<"["<<(*it).first<<"]\n";
		const physics_cfg_keys &k=(*it).second;
		for (physics_cfg_keys::const_iterator it=k.begin();it!=k.end();++it) {
			std::cout<<(*it).first<<"="<<(*it).second<<"\n";
		}
	}
}

int parseconfig(void) {
	physics_cfg_file f=physics_cfg_file_read("test.ini");
	return f.size();
}

std::string randstr(void) {
	std::string s;
	if (rand()%2) s+=" ";
	while (rand()%6!=0) {
		if (rand()%4)
			s+='a'+(rand()%26); // lowercase
		else if (rand()%2)
			s+='A'+(rand()%26); // capitals
		else
			s+='0'+(rand()%10); // digit
	}
	if (rand()%2) s+=" ";
	return s;
}

void make_config(int len)
{
	std::ofstream of("test.ini");
	for (int i=0;i<len;i++) {
		if (rand()%8==0) { // section
			of<<"["<<randstr()<<"]";
		} else { // keyword=value
			of<<randstr()<<"="<<randstr();
		}

		if (rand()%3==0) // comment
			of<<";"<<randstr();
		of<<"\n";
	}
	of.close();
}

int foo(void) {
	make_config(50);
	cat("test.ini");
	physics_cfg_file f=physics_cfg_file_read("test.ini");
	print(f);
	for (int len=1;len<=10000;len*=10) {
		make_config(len);
		printf("%d line ",len);
		print_time("config",parseconfig);
	}
	return 0;
}
#endif
