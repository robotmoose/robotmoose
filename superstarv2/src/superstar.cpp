//Mike Moss
//10/14/2016
//Contains the superstar "database" object (actually does the set/get operations).

#include "superstar.hpp"

#include "auth.hpp"
#include <cmath>
#include <fstream>
#include "json_util.hpp"
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include "string_util.hpp"

//Superstar constructor.
//	Auth file is the file containing authorizations.
//  If the file doesn't exist, superstar assumes there are no auth codes...
//  Backup file is the name of the backup file load saves to.
superstar_t::superstar_t(const std::string& auth_file,const std::string& backup_file):
	auth_file_m(auth_file),backup_file_m(backup_file)
{}

//Replace all multiple slashes with a single slash (hacky...but it works...)
static std::string remove_double_slashes(std::string str)
{
	size_t times=str.size()/2+1;
	for(size_t ii=0;ii<times;++ii)
		str=replace_all(str,"//","/");
	return str;
}

//Turn a path into a vector of strings.
//  "///test//hello//blah/" yields {"test","hello","blah"}.
std::vector<std::string> superstar_t::pathify(std::string path) const
{
	//Remove multiple,leading, and trailing slashes...
	path=remove_double_slashes(strip(path,"/"));

	//Finally tokenize the path...
	return split(path,"/");
}

//Get the value from the "JSON" object path, or null if it doesn't exist.
Json::Value superstar_t::get(const std::string& path) const
{
	//Tokenize path.
	std::vector<std::string> paths(pathify(path));

	//JSON starts at root.
	const Json::Value* obj=&database_m;

	//Traverse paths.
	for(size_t ii=0;ii<paths.size();++ii)
	{
		//If object is an array, change to index instead of str.
		if(obj->isArray())
		{
			std::istringstream istr(paths[ii]);
			size_t index;
			bool was_int=((istr>>index)&&istr.eof());
			if(was_int)
			{
				obj=&((*obj)[(Json::ArrayIndex)index]);
				continue;
			}
		}

		//Path doesn't exist, or it's not an object, return null.
		if(!obj->isObject()||!obj->isMember(paths[ii]))
			return Json::nullValue;

		//Next path.
		obj=&((*obj)[paths[ii]]);
	}

	//Found object, return it.
	return *obj;
}

//Sets value of the "JSON" object path to val.
//  Note, creates full path if path doesn't exist.
//  Note, if existing key are not objects in the path,
//        they will be afterwards... So setting /a/b to foo in
//        object {"a":123} would yield: {"a":{"b":"foo"}}...
//  Note, setting someting to null removes it from the database.
void superstar_t::set(const std::string& path,const Json::Value& val)
{
	//Tokenize path.
	std::vector<std::string> paths(pathify(path));

	//JSON starts at root.
	Json::Value* obj=&database_m;
	Json::Value* par=&database_m;

	//Traverse paths.
	for(size_t ii=0;ii<paths.size();++ii)
	{
		//If object is an array, change to index instead of str.
		if(obj->isArray())
		{
			std::istringstream istr(paths[ii]);
			size_t index;
			bool was_int=((istr>>index)&&istr.eof());
			if(was_int)
			{
				obj=&((*obj)[(Json::ArrayIndex)index]);

				//Array value isn't an object...make it one.
				if(!obj->isObject()&&!obj->isArray())
					*obj=Json::objectValue;

				continue;
			}

			//Index wasn't an int...make the whole array an object...
			*obj=Json::objectValue;
		}

		//Path is not an object, make it one.
		if(!obj->isObject())
			*obj=Json::objectValue;

		//Path doesn't exist, make it.
		if(!obj->isMember(paths[ii]))
			((*obj)[paths[ii]])=Json::objectValue;

		//Next path.
		par=obj;
		obj=&((*obj)[paths[ii]]);
	}

	//Set value of path.
	if(val!=Json::nullValue)
		*obj=val;
	else if(par!=obj&&paths.size()>0)
		par->removeMember(paths[paths.size()-1]);
	else
		*obj=Json::nullValue;
}

//Lists all keys in an object or array.
//  Note, follows javascript standard when dealing with arrays (
//  returns indicies).
Json::Value superstar_t::sub(const std::string& path) const
{
	//Tokenize path.
	std::vector<std::string> paths(pathify(path));

	//JSON starts at root.
	const Json::Value* obj=&database_m;

	//Traverse paths.
	for(size_t ii=0;ii<paths.size();++ii)
	{
		//If object is an array, change to index instead of str.
		if(obj->isArray())
		{
			std::istringstream istr(paths[ii]);
			size_t index;
			bool was_int=((istr>>index)&&istr.eof());
			if(was_int)
			{
				obj=&((*obj)[(Json::ArrayIndex)index]);
				continue;
			}
		}

		//Path doesn't exist, or it's not an object, return [].
		if(!obj->isObject()||!obj->isMember(paths[ii]))
			return Json::arrayValue;

		//Next path.
		obj=&((*obj)[paths[ii]]);
	}

	//Not an object or array...return [].
	if(!obj->isObject()&&!obj->isArray())
		return Json::arrayValue;

	//Might actually have keys/indicies now...
	Json::Value keys(Json::arrayValue);

	//Array value, append indicies to the JSON array...
	if(obj->isArray())
	{
		for(size_t ii=0;ii<obj->size();++ii)
			keys.append((Json::ArrayIndex)ii);
	}

	//Object, append members to the JSON array...
	else
	{
		Json::Value::Members members(obj->getMemberNames());
		for(size_t ii=0;ii<members.size();++ii)
			keys.append(members[ii]);
	}

	//Return sub keys (or []).
	return keys;
}

//Pushes a val into an array located at path and resizes it to len.
//  Arrays here are treated as queues.
//  Similar functionality of set when dealing with existing key names in
//  the given path.
void superstar_t::push(const std::string& path,const Json::Value& val,
	const Json::Value& len)
{
	//Tokenize path.
	std::vector<std::string> paths(pathify(path));

	//JSON starts at root.
	Json::Value* obj=&database_m;

	//Traverse paths.
	for(size_t ii=0;ii<paths.size();++ii)
	{
		//If object is an array, change to index instead of str.
		if(obj->isArray())
		{
			std::istringstream istr(paths[ii]);
			size_t index;
			bool was_int=((istr>>index)&&istr.eof());
			if(was_int)
			{
				obj=&((*obj)[(Json::ArrayIndex)index]);

				//Array value isn't an object...make it one.
				if(!obj->isObject()&&!obj->isArray())
					*obj=Json::objectValue;

				continue;
			}

			//Index wasn't an int...make the whole array an object...
			*obj=Json::objectValue;
		}


		//Path is not an object, make it one.
		if(!obj->isObject())
			*obj=Json::objectValue;

		//Path doesn't exist, make it.
		if(!obj->isMember(paths[ii]))
			((*obj)[paths[ii]])=Json::objectValue;

		//Next path.
		obj=&((*obj)[paths[ii]]);
	}

	//Not an array...make it an array.
	if(!obj->isArray())
		*obj=Json::arrayValue;

	//Don't bother building the copy array if length 0 (see notes below for why).
	if(!len.isNull()&&len.asUInt()==0)
		return;

	//Append value
	obj->append(val);

	//Limit array size...
	//  Note, gave up trying to use min/std::min...templates man...
	//  Note, jsoncpp has a VERY expensive removeMember...this is WAY cheaper...
	Json::Value new_array=Json::arrayValue;
	size_t min_size=obj->size();
	if(!len.isNull()&&min_size>len.asUInt())
		min_size=len.asUInt();
	if(min_size>1000)
		min_size=1000;
	for(Json::ArrayIndex ii=0;ii<min_size;++ii)
		new_array.append((*obj)[(Json::ArrayIndex)(obj->size()-min_size+ii)]);
	*obj=new_array;
}
#include <iostream>
//Changes auth code for a given path to the given value.
//  Returns whether new auth was set or not.
//  Note, min auth code length is 4 characters (SEE NOTE BELOW).
//  Note, "-" indicates to REMOVE auth code and "!" indicates to set path to IMMUTABLE.
//  Note, valid characters are ASCII 33-126 inclusive.
//  Note, relies on HTTPS for secure transport.
//  Note, traverses auth until a matching code is found for a path.
//        This is NOT create new auth codes, only changes existing ones.
//  Note, RELIES ON AUTH_CHECK TO HANDLE IMMUTABLE ENTRIES.
bool superstar_t::change_auth(std::string path,const Json::Value& value)
{
	//Check auth length.
	std::string new_auth_str="";
	if(!value.isNull())
		new_auth_str=value.asString();
	if(new_auth_str.size()<4&&new_auth_str!="!"&&new_auth_str!="-"&&new_auth_str.size()>0)
		return false;

	//Check for invalid characters.
	for(size_t ii=0;ii<new_auth_str.size();++ii)
		if(new_auth_str[ii]<33||new_auth_str[ii]>126)
			return false;

	//Remove multiple,leading, and trailing slashes...
	path=remove_double_slashes(strip(path,"/"));
	if(path.size()==0)
		path="/";

	//Try to open auth file.
	std::ifstream ifstr(auth_file_m.c_str());

	//Open auth file and parse passwords in line based format "PATH PASSWORD" (without quotes).
	std::vector<std::pair<std::string,std::string> > auths;
	if(ifstr.good())
	{
		std::string line;
		while(std::getline(ifstr,line))
		{
			std::istringstream istr(line);

			//Parse path from line...
			std::string path;
			if(!(istr>>path))
				continue;

			//Parse auth from line...
			std::string auth;
			if(!(istr>>auth))
				auth="";

			auths.push_back(std::pair<std::string,std::string>(path,auth));
		}
		ifstr.close();
	}

	//Open auth file in write mode.
	std::ofstream ofstr(auth_file_m.c_str());
	if(!ofstr.good())
	{
		std::cout<<"ERROR!"<<std::endl;
		return false;
	}

	//Write all auths and replace the one with our path with the new auth code.
	bool found=false;
	for(size_t ii=0;ii<auths.size();++ii)
	{
		//If path for auth change and current path are the same, change.
		std::string cur_path=remove_double_slashes(strip(auths[ii].first,"/"));
		if(cur_path==path)
		{
			if(new_auth_str!="-")
				ofstr<<auths[ii].first<<" "<<new_auth_str<<std::endl;
			found=true;
		}
		else
		{
			ofstr<<auths[ii].first<<" "<<auths[ii].second<<std::endl;
		}
	}

	//Wasn't found in file, make it.
	if(!found&&new_auth_str!="-")
		ofstr<<path<<" "<<new_auth_str<<std::endl;
	ofstr.close();

	return true;
}

//Authenticates path with opts with the passed auth object.
//  Variable was_immutable indicates whether or not the auth was immutable or not.
//  Note, expects a string or null in the auth object.
//  Note, PATH IS STRIPPED OF ALL PRECEEDING /'s and ENDING /'s,
//        AND MULTIPLE /'s ARE CHANGED TO A SINGLE /.
bool superstar_t::auth_check(std::string path,const std::string& opts,
	const Json::Value& auth,bool& was_immutable)
{
	//Remove multiple,leading, and trailing slashes...
	path=remove_double_slashes(strip(path,"/"));

	//Auth as string..
	std::string auth_str="";
	if(!auth.isNull())
		auth_str=auth.asString();

	//Try to open auth file.
	std::ifstream ifstr(auth_file_m.c_str());

	//No auth file...no authentication.
	if(!ifstr.good())
		return true;

	//Open auth file and parse passwords in line based format "PATH PASSWORD" (without quotes).
	std::string line;
	std::map<std::string,std::string> table;
	size_t table_count=0;
	while(std::getline(ifstr,line))
	{
		std::istringstream istr(line);

		//Parse path from line...
		std::string path;
		if(!(istr>>path))
			continue;
		path=remove_double_slashes(strip(path,"/"));

		//Parse auth from line...
		std::string auth;
		if(!(istr>>auth))
			auth="";

		table[path]=auth;
		++table_count;
	}
	ifstr.close();

	//Empty auth file
	if(table_count==0)
		return true;

	//Iteratively go up through entire tree structure.
	std::string parent_path(path);
	was_immutable=false;
	bool authenticated=false;
	bool authenticated_with_bang=false;
	while(true)
	{
		bool newly_authenticated=false;
		bool not_bang=false;
		bool in_file=false;

		//if path in table.
		if(table.count(parent_path)>0)
		{
			//In table then in file.
			in_file=true;

			//Immutable check.
			not_bang=(table[parent_path]!="!");
			if(!not_bang)
				was_immutable=true;

			//If empty auth or matching auth, authenticate.
			if(table[parent_path].size()<=0||
				to_hex_string(hmac_sha256(table[parent_path],path+opts))==auth_str)
				newly_authenticated=true;

			//If "!" auth, authenticate.
			if(table[parent_path]=="!")
				authenticated_with_bang=true;
		}

		//Transitioning from a level of immutable to not immutable.
		if(newly_authenticated&&!authenticated&&in_file&&not_bang)
			was_immutable=false;

		//Got authenticated.
		if(newly_authenticated)
			authenticated=true;

		//Hit end of tree structure, break.
		if(parent_path.size()==0)
			break;

		//Strip off top level.
		while(parent_path.size()>0&&parent_path[parent_path.size()-1]!='/')
			parent_path=path.substr(0,parent_path.size()-1);
		parent_path=strip_end(parent_path,"/");
	}

	return (authenticated||authenticated_with_bang);
}

//Loads from either an old style binary file (superstar v1).
//Or from a JSON file (superstar v2).
bool superstar_t::load()
{
	//Empty backup file...return false...
	if(backup_file_m.size()==0)
		return false;

	//Try to load old version...
	if(load_v1())
		return true;

	//Try to open file.
	char buffer;
	std::ifstream istr(backup_file_m.c_str(),std::ios_base::in|std::ios_base::binary);
	istr.unsetf(std::ios_base::skipws);

	//File doesn't exist, this is ok...
	if(!istr)
		return false;

	//Read all data in file...
	std::string data="";
	while(istr>>buffer)
		data+=buffer;
	istr.close();

	//Try to load as JSON...
	try
	{
		set("/",JSON_deserialize(data));
		return true;
	}

	//File isn't json...bad...
	catch(...)
	{
		throw std::runtime_error("Malformed backup file \""+backup_file_m+"\"...");
	}
}

//Loads from old style binary files (superstar v1).
bool superstar_t::load_v1()
{
	//Empty backup file...return false...
	if(backup_file_m.size()==0)
		return false;

	//Try to open file.
	std::ifstream istr(backup_file_m.c_str(),std::ios_base::in|std::ios_base::binary);
	istr.unsetf(std::ios_base::skipws);
	if(!istr)
		return false;

	//Read size of data.
	uint64_t array_length=0;
	istr.read((char*)&array_length,sizeof(uint64_t));

	//Create an empty superstar...
	superstar_t superstar_new("","");

	//In case too big of strings are tried to be read...
	try
	{
		//Read each entry...
		for(size_t ii=0;ii<array_length;++ii)
		{
			//Get key...
			uint64_t key_size=0;
			std::string key_str;
			if(!istr.read((char*)&key_size,sizeof(uint64_t)))
				return false;
			key_str.resize(key_size);
			if(!istr.read(&key_str[0],key_size))
				return false;

			//Get value...
			uint64_t value_size=0;
			std::string value_str;
			if(!istr.read((char*)&value_size,sizeof(uint64_t)))
				return false;
			value_str.resize(value_size,'\0');
			if(!istr.read(&value_str[0],value_size))
				return false;

			//Decode JSON objects as JSON.
			Json::Value value;
			try
			{
				value=JSON_deserialize(value_str);
			}
			catch(...)
			{
				value=value_str;
			}

			superstar_new.set(key_str,value);
		}
		istr.close();
	}

	//Strings were too big...
	catch(...)
	{
		istr.close();
		return false;
	}

	//Overwrite database...
	database_m=superstar_new.database_m;
	return true;
}

//Saves to a JSON file (superstar v2 format).
bool superstar_t::save()
{
	//Empty backup file...return false...
	if(backup_file_m.size()==0)
		return false;

	//Save database...
	std::string database_json(JSON_serialize(get("/")));
	std::ofstream ostr(backup_file_m.c_str(),std::ios_base::out|std::ios_base::binary);
	bool saved=(bool)(ostr<<database_json);
	ostr.close();
	return saved;
}