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
		//Superstar constructor.
		//	Auth file is the file containing authorizations.
		//  If the file doesn't exist, superstar assumes there are no auth codes...
		//  Backup file is the name of the backup file load saves to.
		superstar_t(const std::string& auth_file,const std::string& backup_file);

		//Turn a path into a vector of strings.
		//  "///test//hello//blah/" yields {"test","hello","blah"}.
		std::vector<std::string> pathify(std::string path) const;

		//Get the value from the "JSON" object path, or null if it doesn't exist.
		Json::Value get(const std::string& path) const;

		//Sets value of the "JSON" object path to val.
		//  Note, Creates full path if path doesn't exist.
		//  Note, If existing key are not objects in the path,
		//        they will be afterwards... So setting /a/b to foo in
		//        object {"a":123} would yield: {"a":{"b":"foo"}}...
		void set(const std::string& path,const Json::Value& val);

		//Lists all keys in an object or array.
		//  Note, follows javascript standard when dealing with arrays (
		//  returns indicies).
		Json::Value sub(const std::string& path) const;

		//Pushes a val into an array located at path and resizes it to len.
		//  Arrays here are treated as queues.
		//  Similar functionality of set when dealing with existing key names in
		//  the given path.
		void push(const std::string& path,const Json::Value& val,const Json::Value& len);

		//Authenticates path with opts with the passed auth object
		//  Note, expects a string or null in the auth object.
		//  Note, recursive function, passing the original path around in
		//        recursive_path...
		//  Note, PATH IS STRIPPED OF ALL PRECEEDING /'s and ENDING /'s,
		//        AND MULTIPLE /'s ARE CHANGED TO A SINGLE /.
		bool auth_check(std::string path,const std::string& opts,
			const Json::Value& auth,std::string recursive_path="");

		//Loads from either an old style binary file (superstar v1).
		//Or from a JSON file (superstar v2).
		//  Note, THIS OVERWRITES THE CURRENTLY LOADED DATABASE.
		bool load();

		//Loads from old style binary files (superstar v1).
		//  Note, THIS OVERWRITES THE CURRENTLY LOADED DATABASE.
		bool load_v1();

		//Saves to a JSON file (superstar v2 format).
		//  Note, THIS OVERWRITES THE OLD DATABASE FILE.
		bool save();

	private:
		std::string auth_file_m;
		std::string backup_file_m;
		Json::Value database_m;
};

#endif