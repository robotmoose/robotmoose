#ifndef SUPERSTAR_HPP_INCLUDED
#define SUPERSTAR_HPP_INCLUDED

#include <cstddef>
#include <string>
#include "jsoncpp/json.h"
#include <vector>


class superstar_t {
	public:
		superstar_t(std::string url);
		void get(std::string path, void(*successcb)(Json::Value)=0, void(*errorcb)(Json::Value)=0);
		void set(std::string path, Json::Value value, std::string auth, 
			void(*successcb)(Json::Value)=0, void(*errorcb)(Json::Value)=0);
		void sub(std::string path, void(*successcb)(Json::Value)=NULL, void(*errorcb)(Json::Value)=NULL);
		void push(std::string path, Json::Value value, int length, std::string auth="", 
			void(*successcb)(Json::Value)=NULL, void(*errorcb)(Json::Value)=NULL);
		void get_next(std::string path, void(*successcb)(Json::Value)=NULL, void(*errorcb)(Json::Value)=NULL);
		void change_auth(std::string path, Json::Value value, std::string auth="", 
			void(*successcb)(Json::Value)=NULL, void(*errorcb)(Json::Value)=NULL);
		Json::Value build_skeleton_request(std::string method, std::string path, std::string opts=NULL);
		void add_request(Json::Value & request, void(*successcb)(Json::Value)=NULL, void(*errorcb)(Json::Value)=NULL);
		std::string pathify(std::string path);
		void flush();
	private:
		std::string superstar;
		std::vector<Json::Value> queue;
		std::vector<Json::Value> old_queue;
};

#endif // SUPERSTAR_HPP_INCLUDED