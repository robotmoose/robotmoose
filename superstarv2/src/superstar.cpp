//Mike Moss
//07/09/2016
//Contains the superstar "database" object (actually does the set/get operations).
//FIXME:  Needs authentication.

#include "superstar.hpp"

#include <sstream>
#include "string_util.hpp"

//Turn a path into a vector of strings.
//  "///test//hello//blah/" yields {"test","hello","blah"}.
std::vector<std::string> superstar_t::pathify(std::string path) const
{
	//Remove leading and trailing slashes...
	path=strip(path,"/");

	//Replace all multiple slashes with a single slash (hacky...but it works...)
	for(size_t ii=0;ii<path.size()/2+1;++ii)
		path=replace_all(path,"//","/");

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
//  Creates full path if path doesn't exist.
//  If existing key are not objects in the path, they will be afterwards...
//    So setting /a/b to foo on object {"a":123} would yield {"a":{"b":"foo"}}.
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
// Note, follows javascript standard when dealing with arrays (returns indicies).
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
//  Similar functionality of set when dealing with existing key names in the given path.
void superstar_t::push(const std::string& path,const Json::Value& val,size_t len)
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
	if(len==0)
		return;

	//Append value
	obj->append(val);

	//Limit array size...
	//  Note, gave up trying to use min/std::min...templates man...
	//  Note, jsoncpp has a VERY expensive removeMember...this is WAY cheaper...
	Json::Value new_array=Json::arrayValue;
	size_t min_size=obj->size();
	if(min_size>len)
		min_size=len;
	if(min_size>1000)
		min_size=1000;
	for(Json::ArrayIndex ii=0;ii<min_size;++ii)
		new_array.append((*obj)[(Json::ArrayIndex)(obj->size()-min_size+ii)]);
	*obj=new_array;
}