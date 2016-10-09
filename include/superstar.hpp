//Mike Moss
//09/24/2016
//Contains client code to get requests from a superstar server.

#ifndef SUPERSTAR_HPP
#define SUPERSTAR_HPP

#include <iostream>
#include <jsoncpp/json.h>
#include <vector>

typedef void(*success_cb_t)(const Json::Value& value);
typedef void(*error_cb_t)(const Json::Value& value);

struct starequest_t
{
	Json::Value request;
	success_cb_t success_cb;
	error_cb_t error_cb;
};

//Superstar object.
//  Variable queue to store requests in until .flush is called.
class superstar_t
{
	public:
		superstar_t(const std::string& url);

		//Gets the value of path.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		void get(std::string path,success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Sets path to the value using the given auth.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		void set(std::string path,Json::Value value,const std::string& auth="",success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Gets sub keys of the given path.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		void sub(std::string path,success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Pushes the given value onto path using the given auth.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		//  Note, if the path is not an array, it will be after this.
		void push(std::string path,Json::Value value,const size_t& length,const std::string& auth="",success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Gets the value of path when it changes.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		//  Note, python version is unique because it is BLOCKING.
		void get_next(std::string path,const std::string& last_hash,
			success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Changes auth for the given path and auth to the given value.
		//  Calls success_cb on success with the server response.
		//  Calls error_cb on error with the server error object (as per spec).
		void change_auth(std::string path,Json::Value value,const std::string& auth="",success_cb_t success_cb=NULL,error_cb_t error_cb=NULL);

		//Replaces multiple slashes in path with a single slash.
		//  Removes all leading&&trailing slashes.
		std::string pathify(std::string path);

		//Builds a basic jsonrpc request with given method.
		//  Adds path as path&&opts as opts to the params object.
		//  Note, opts is optional.
		Json::Value build_skeleton_request(const std::string& method,std::string path,Json::Value opts=Json::nullValue);

		//Builds HMACSHA256 for given path&&request object with given auth.
		void build_auth(std::string& path,Json::Value& request,const std::string& auth);

		//Adds a build request into the batch queue.
		//  Note, won't be sent until .flush() is called.
		void add_request(Json::Value& request,success_cb_t success_cb,success_cb_t error_cb);

		//Builds the batch request object and clears out the current queue.
		void flush();

		//Function to handle errors...
		void handle_error(error_cb_t error_cb,const Json::Value& error);

	private:
		std::string superstar;
		std::vector<starequest_t> queue;

};

#endif