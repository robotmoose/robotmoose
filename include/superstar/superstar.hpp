#ifndef SUPERSTAR_HPP_INCLUDED
#define SUPERSTAR_HPP_INCLUDED

#include <cstddef>
#include <cstdlib>
#include <string>
#include "jsoncpp/json.h"
#include "json_util.hpp"
#include <vector>
#include <map>


class superstar_t {
	public:
		superstar_t(std::string url);
		void get(std::string path, void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void set(std::string path, Json::Value value, std::string auth="", 
			void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void sub(std::string path, void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void push(std::string path, Json::Value value, int length, std::string auth="", 
			void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void get_next(std::string path, void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void change_auth(std::string path, Json::Value value, std::string auth="", 
			void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		void flush();
	private:
		std::string superstar;
		std::vector<Json::Value> queue, old_queue;
		typedef void (*callback)(Json::Value);
		std::map<std::string, callback> callbacks;

		Json::Value build_skeleton_request(std::string method, std::string path, std::string opts="");
		void add_request(Json::Value & request, void(*success_cb)(Json::Value)=NULL, void(*error_cb)(Json::Value)=NULL);
		std::string pathify(std::string path);
		void handle_error(Json::Value request, Json::Value error);
};

#endif // SUPERSTAR_HPP_INCLUDED