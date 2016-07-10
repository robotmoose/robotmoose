//Mike Moss
//07/09/2016
//Contains the superstar "database" object (actually does the set/get operations).
//FIXME:  Needs authentication.

#include "superstar.hpp"

#include "auth.hpp"
#include <fstream>
#include <sstream>
#include "string_util.hpp"

//Superstar constructor, auth file is the file containing authorizations.
//  If the file doesn't exist, superstar assumes there are no auth codes...
superstar_t::superstar_t(const std::string& auth_file):auth_file_m(auth_file)
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
//  Note, Creates full path if path doesn't exist.
//  Note, If existing key are not objects in the path,
//        they will be afterwards... So setting /a/b to foo in
//        object {"a":123} would yield: {"a":{"b":"foo"}}...
void superstar_t::set(const std::string& path,const Json::Value& val)
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

	//Set value of path.
	*obj=val;
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

//Authenticates path with opts with the passed auth object
//  Note, expects a string or null in the auth object.
//  Note, recursive function, passing the original path around in
//        recursive_path...
//  Note, PATH IS STRIPPED OF ALL PRECEEDING /'s and ENDING /'s,
//        AND MULTIPLE /'s ARE CHANGED TO A SINGLE /.
bool superstar_t::auth_check(std::string path,const std::string& opts,
	const Json::Value& auth,std::string recursive_path)
{
	//Remove multiple,leading, and trailing slashes...
	path=remove_double_slashes(strip(path,"/"));

	//This is a recursive function, need to pass the path around...
	if(recursive_path.size()==0)
		recursive_path=path;

	//Try to open auth file.
	std::ifstream fstr(auth_file_m);

	//No passwords at all...authenticated...
	if(!fstr.good())
		return true;

	//Open auth file and parse passwords in line based format "PATH PASSWORD" (without quotes).
	bool found=false;
	std::string pass;
	while(std::getline(fstr,pass))
	{
		//Read whole line into pass...need to separate into path and pass...
		std::istringstream istr(pass);
		std::string challenge_path;
		istr>>challenge_path;

		//Strip beginning and ending slashes to make path checks easier...
		challenge_path=remove_double_slashes(strip(challenge_path,"/"));

		//Found path, grab password...
		if(challenge_path==path)
		{
			//Empty password...
			if(!(istr>>pass))
				pass="";
			found=true;
			break;
		}
	}
	fstr.close();

	//No password found...
	if(!found)
	{
		//Remove top level of path.
		std::string next_path=path;
		while(next_path.size()>0&&next_path[next_path.size()-1]!='/')
			next_path=next_path.substr(0,next_path.size()-1);

		//Remove multiple,leading, and trailing slashes...
		next_path=remove_double_slashes(strip(next_path,"/"));

		//If top level path isn't blank and not the same as the old one, try it's auth.
		if(next_path!=path)
			return auth_check(next_path,opts,auth,recursive_path);

		//Else no auth, authenticated.
		return true;
	}

	//Blank password is the same as no password.
	if(pass.size()<=0)
		return true;

	//Perform HMAC.
	std::string auth_str="";
	if(!auth.isNull())
		auth_str=auth.asString();
	return (auth_str==to_hex_string(hmac_sha256(pass,recursive_path+":"+opts)));
}