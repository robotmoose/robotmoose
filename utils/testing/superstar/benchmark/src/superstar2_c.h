//superstar2_c.h
//Ben Bettisworth
//A "single" file client for superstar v2
//and thats all the documenation you get
#pragma once
#include "../include/hash/hmac.h"
#include "../include/hash/sha256.h"
#include "../include/jsoncpp/json.h"
#include <string>
#include <sstream>
#include <stdio.h>
#include <curl/curl.h>
#include <vector>
#include <memory>

void response_to_string(void* ptr, size_t size, size_t nmemb, std::string* s){
    s->resize(size*nmemb);
    for(size_t i = 0;i < size*nmemb; ++i){
        (*s)[i] = ((char*)ptr)[i];
    }
}

class superstar_session_t{
    public:
        superstar_session_t(const std::string& url):_id(0), _url(url){ make_writer(); }
        superstar_session_t(const std::string& url, const std::string& key): _id(0), _url(url), _key(key){ make_writer(); } 
        void set_key(std::string k){ _key = k; }

        std::string get(std::string path){
            Json::Value post_json;
            post_json["jsonrpc"] = "2.0";
            post_json["method"] = "get";
            post_json["id"] = _id++;
            post_json["params"]["path"] = path;
            
            return send_json(post_json);
        }

        std::string set(std::string path, Json::Value value){
            Json::Value post_json;
            post_json["jsonrpc"] = "2.0";
            post_json["method"] = "set";
            post_json["id"] = _id++;
            post_json["params"]["path"] = path;

            std::ostringstream opts_string;
            Json::Value opts_value;
            opts_value["value"] = value;
            _writer->write(opts_value, &opts_string);

            post_json["params"]["opts"] = opts_string.str();
            post_json["params"]["auth"] = hmac<SHA256>(opts_string.str(), _key);

            return send_json(post_json);
        }

        std::string sub(std::string path){
            Json::Value post_json;
            post_json["jsonrpc"] = "2.0";
            post_json["method"] = "sub";
            post_json["id"] = _id++;
            post_json["params"] = Json::Value();
            post_json["params"]["path"] = path;

            return send_json(post_json);
        }

        std::string push(std::string path, Json::Value value, int len=0){
            Json::Value post_json;
            post_json["jsonrpc"] = "2.0";
            post_json["method"] = "set";
            post_json["id"] = _id++;
            post_json["params"]["path"] = path;

            std::ostringstream opts_string;
            Json::Value opts_value;
            opts_value["value"] = value;
            opts_value["length"] = len;
            _writer->write(opts_value, &opts_string);

            post_json["params"]["opts"] = opts_string.str();
            post_json["params"]["auth"] = hmac<SHA256>(opts_string.str(), _key);

            return send_json(post_json);
        }

    private:
        void make_writer(){
            Json::StreamWriterBuilder swb;
            swb["comentStyle"] = "None";
            swb["indentation"] = "";
            std::unique_ptr<Json::StreamWriter> fw(swb.newStreamWriter());
            _writer = std::unique_ptr<Json::StreamWriter>(swb.newStreamWriter());
        }

        std::string send_json(const Json::Value& json){
            CURL *curl;
            curl = curl_easy_init();

            std::stringstream post_stream;
            _writer->write(json, &post_stream);
            std::string post_data = post_stream.str();

            std::string resp;
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,response_to_string);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());

                CURLcode res;
                res = curl_easy_perform(curl);
                if(res != CURLE_OK)
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(curl);
            }
            return resp;
        }

        int _id;
        std::string _url;
        std::string _key;
        std::unique_ptr<Json::StreamWriter> _writer;
};
