//Mike Moss
//07/09/2016
//Contains the superstar "database" object (actually does the set/get operations).

#ifndef SUPERSTAR_HPP
#define SUPERSTAR_HPP

#include <string>
#include <vector>
#include <jsoncpp/json.h>

class superstar_t
{
	public:
		//Superstar constructor, auth file is the file containing authorizations.
		//  If the file doesn't exist, superstar assumes there are no auth codes...
		superstar_t(const std::string& auth_file);

		//Turn a path into a vector of strings.
		//  "///test//hello//blah/" yields {"test","hello","blah"}.
		std::vector<std::string> pathify(std::string path) const;

		//Get the value from the "JSON" object path, or null if it doesn't exist.
		Json::Value get(const std::string& path) const;

		//Sets value of the "JSON" object path to val.
		//  Creates full path if path doesn't exist.
		//  If existing key are not objects in the path, they will be afterwards...
		//    So setting /a/b to foo on object {"a":123} would yield {"a":{"b":"foo"}}.
		void set(const std::string& path,const Json::Value& val);

		//Lists all keys in an object or array.
		// Note, follows javascript standard when dealing with arrays (returns indicies).
		Json::Value sub(const std::string& path) const;

		//Pushes a val into an array located at path and resizes it to len.
		//  Arrays here are treated as queues.
		//  Similar functionality of set when dealing with existing key names in the given path.
		void push(const std::string& path,const Json::Value& val,const Json::Value& len);

		//Authenticates path with opts with the passed auth object (expects a string or null).
		//  FIXME:  Authentication is hardcoded to 123...
		bool auth_check(const std::string& path,const std::string& opts,const Json::Value& auth);

	private:
		std::string auth_file_m;
		Json::Value database_m;
};

#endif