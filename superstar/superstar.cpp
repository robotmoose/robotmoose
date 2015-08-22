/**
  Superstar: the "star" in the center of our network topology.
  This is an HTTP server on a public IP, used to connect the pilot
  and robot, who both may live behind firewalls.



  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-10-02 (Public Domain)
*/
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <stdarg.h>
#include "mongoose/mongoose.h" /* central webserver library */
#include <stdint.h>

#include "osl/sha2_auth.h" /* for authentication */
#include "osl/sha2.cpp" /* for easier linking */


#ifdef USE_CPP_11
#include <chrono>
#include <thread>

int64_t millis()
{
	auto system_time=std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(system_time).count();
}

#else
#include <time.h>

int64_t millis() {
	int64_t now=time(0); // seconds
	return 1000*now;  // HOPE YOU DIDN'T NEED MILLISECOND RESOLUTION!
}

#endif

std::string ADDRESS="0.0.0.0:8081";
const std::string backup_filename="db.bak";
const int64_t backup_time=5000;
int64_t old_time=millis();

/// Utility: integer to string
std::string my_itos(int i) {
	char buf[100];
#ifdef _WIN32
#define snprintf _snprintf
#endif
	snprintf(buf,sizeof(buf),"%d",i);
	return buf;
}

/**
  Simple mongoose utility function: send back raw JSON reply.
  This is designed to be minimal, and easy to parse in JavaScript or C++.
*/
int send_json(struct mg_connection *conn,std::string json)
{
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/json\r\n"
		"Content-Length: %ld\r\n"        // Always set Content-Length
		"\r\n"
		"%s",
		json.size(), json.c_str());
	return MG_TRUE;
}

/**
  This is the key-value "database" of everything stored by superstar.
  Eventually it would be nice to persist this to disk, and keep history, although
  that's probably not needed for robotics piloting, which tends to be same-day.
*/
class superstar_db_t {
private:
	typedef std::map<std::string /* path */, std::string /* value */> db_t;
	db_t db;
public:
	superstar_db_t() {}

	void save(const std::string& filename)
	{
		std::ofstream ostr(filename.c_str(),std::ios_base::out|std::ios_base::binary);
		ostr.unsetf(std::ios_base::skipws);

		if(!ostr)
			throw std::runtime_error("superstar_db_t::save() - Could not open file named \""+
				filename+"\" for saving.");

		uint64_t array_length=db.size();
		ostr.write((char*)&array_length,sizeof(uint64_t));

		for(db_t::const_iterator entry=db.begin();entry!=db.end();++entry)
		{
			uint64_t key_size=entry->first.size();
			std::string key_str=entry->first;
			if(!ostr.write((char*)&key_size,sizeof(uint64_t)))
				throw std::runtime_error("superstar_db_t::save() - Error writeing key size from file named \""+
					filename+"\".");
			if(!ostr.write(&key_str[0],key_size))
				throw std::runtime_error("superstar_db_t::save() - Error writeing key from file named \""+
					filename+"\".");

			uint64_t data_size=entry->second.size();
			std::string data_str=entry->second;
			if(!ostr.write((char*)&data_size,sizeof(uint64_t)))
				throw std::runtime_error("superstar_db_t::save() - Error writeing data size from file named \""+
					filename+"\" for loading.");
			if(!ostr.write(&data_str[0],data_size))
				throw std::runtime_error("superstar_db_t::save() - Error writeing data from file named \""+
					filename+"\".");
		}

		ostr.close();
	}

	void load(const std::string& filename)
	{
		std::ifstream istr(filename.c_str(),std::ios_base::in|std::ios_base::binary);
		istr.unsetf(std::ios_base::skipws);

		if(!istr)
			throw std::runtime_error("superstar_db_t::load() - Could not open file named \""+
				filename+"\" for loading.");

		uint64_t array_length=0;
		istr.read((char*)&array_length,sizeof(uint64_t));
		db_t db_temp;

		for(size_t ii=0;ii<array_length;++ii)
		{
			uint64_t key_size=0;
			std::string key_str;
			if(!istr.read((char*)&key_size,sizeof(uint64_t)))
				throw std::runtime_error("superstar_db_t::load() - Error reading key size from file named \""+
					filename+"\".");
			key_str.resize(key_size);
			if(!istr.read(&key_str[0],key_size))
				throw std::runtime_error("superstar_db_t::load() - Error reading key from file named \""+
					filename+"\".");

			uint64_t data_size=0;
			std::string data_str;
			if(!istr.read((char*)&data_size,sizeof(uint64_t)))
				throw std::runtime_error("superstar_db_t::load() - Error reading data size from file named \""+
					filename+"\" for loading.");
			data_str.resize(data_size);
			if(!istr.read(&data_str[0],data_size))
				throw std::runtime_error("superstar_db_t::load() - Error reading data from file named \""+
					filename+"\".");

			db_temp[key_str]=data_str;
		}

		istr.close();
		db=db_temp;
	}

	/**
	  Overwrite the current value in the database with this new value.
	*/
	void set(std::string path,const std::string &new_value)
	{
		if(path.size()>0&&path[path.size()-1]!='/')
			path+="/";

		if(path.size()==0)
			path+="/";

		if(new_value=="")
		{
			std::cout<<"Deleting entry \""<<path.substr(0,path.size()-1)<<"\"...";
			db_t::iterator iter=db.find(path);

			if(iter!=db.end())
			{
				std::cout<<"success."<<std::endl;
				db.erase(iter);
			}
			else
			{
				std::cout<<"fail (does not exist)."<<std::endl;
			}
		}
		else
		{
			std::cout<<"Setting entry "<<path.substr(0,path.size()-1)<<" to "<<new_value<<std::endl;
			db[path]=new_value;
		}
	}

	/**
	  Read the latest value from the database.
	*/
	const std::string& get(std::string path)
	{
		if(path.size()>0&&path[path.size()-1]!='/')
			path+="/";

		db_t::iterator iter=db.find(path);
		if (iter!=db.end()) {
			return (*iter).second; // i.e., db[path];
		}
		else {
			static const std::string empty="";
			return empty;
		}
	}

	/**
	  Return a comma-separated JSON array of quoted substrings
	  in the database matching this prefix.
	*/
	std::string substrings(std::string path_prefix)
	{
		if(path_prefix.size()>0&&path_prefix[path_prefix.size()-1]!='/')
			path_prefix+="/";

		std::string list="";
		std::string last="";
		for (db_t::const_iterator c=db.begin();c!=db.end();++c)
		{
			std::string path=(*c).first;
			if (0==path.compare(0,path_prefix.size(),path_prefix))
			{ // path matches prefix
				path=path.substr(path_prefix.size()); // trim prefix
				// trim beyond forward slash
				size_t slash=path.find('/');
				if (slash!=std::string::npos) path.resize(slash);

				if (last!=path) {
					if (list.size()==0) list+="[\""+path;
					else list+="\",\""+path;
					last=path;
				}
			}
		}
		if (list.size()>0) return list+"\"]";
		else return "[]";
	}

	std::string sublinks(std::string path_prefix)
	{
		if(path_prefix.size()>0&&path_prefix[path_prefix.size()-1]!='/')
			path_prefix+="/";

		std::string list="";
		std::string last="";
		for (db_t::const_iterator c=db.begin();c!=db.end();++c)
		{
			std::string path=(*c).first;
			std::string prefix=path;
			if (0==path.compare(0,path_prefix.size(),path_prefix))
			{ // path matches prefix
				path=path.substr(path_prefix.size()); // trim prefix
				// trim beyond forward slash
				size_t slash=path.find('/');
				if (slash!=std::string::npos) path.resize(slash);

				if(last!=path)
				{
					list+="<a href=\"/superstar/"+
						path_prefix+path+"\">"+path+"</a> ";
					last=path;
				}
			}
		}

		return list;
	}
};

superstar_db_t superstar_db;

/**
  Return true if a write to this starpath is allowed.
*/
bool write_is_authorized(const std::string &starpath,
	const std::string &new_data,const std::string &new_auth)
{
// Read password from file of passwords
	std::ifstream f(("auth/"+starpath).c_str());
	if (!f.good()) return true; // no such auth file exists.
	std::string pass; std::getline(f,pass);

// If we get here, there is a password.  Verify there is an authentication code.
	if (new_auth.size()<=0) return false; // need authentication code

	// FIXME: extract sequence number (read JSON in new_data?)
	int seq=0;
	std::string alldata=pass+":"+starpath+":"+new_data+":"+my_itos(seq);

// Check to see what auth code should be
	std::string should_auth=getAuthCode<SHA256>(alldata);
	if (should_auth!=new_auth) return false; // authentication mismatch
	else return true;
}

// This function will be called by mongoose on every new request.
int http_handler(struct mg_connection *conn, enum mg_event ev) {
  if (ev==MG_AUTH) return true; // everybody's authorized
  if (ev!=MG_REQUEST) return false; // not a request? not our problem
  // else it's a request

  std::string remote_ip(conn->remote_ip);

  if(remote_ip=="127.0.0.1")
  {
		// std::cout<<"Local IP detected, attempting to get remote..."<<std::flush;

		for(int ii=0;ii<conn->num_headers;++ii)
		{
			if(std::string(conn->http_headers[ii].name)=="X-Forwarded-For")
			{
				remote_ip=conn->http_headers[ii].value;
				// std::cout<<"success, X-Forwarded-For header found."<<std::endl;
				break;
			}
		}

		// if(remote_ip=="127.0.0.1")
		//	std::cout<<"failed."<<std::endl;
  }

  printf("Incoming request: client %s:%d, URI %s\n",remote_ip.c_str(),conn->remote_port,conn->uri);

  const char *prefix="/superstar/";
  if (strncmp(conn->uri,prefix,strlen(prefix))!=0)
  	return MG_FALSE; // file fallback
  std::string starpath(&conn->uri[strlen(prefix)]);

  std::string query="";
  if (conn->query_string) query=conn->query_string;

  std::string content="<HTML><BODY>Hello from mongoose!  "
  "I see you're using source IP "+remote_ip+" and port "+my_itos(conn->remote_port)+"\n";
  content+="<P>Superstar path: "+starpath+"\n";

  enum {NBUF=32767}; // maximum length for JSON data being set
  char buf[NBUF];
  if (0<=mg_get_var(conn,"set",buf,NBUF)) { /* writing new value */
  	std::string newval(buf);
  	char sentauth[NBUF];
  	if (0>mg_get_var(conn,"auth",sentauth,NBUF)) {
  		sentauth[0]=0; // empty authentication string
  	}
  	if (write_is_authorized(starpath,newval,sentauth)) {
		content+="<P>Setting new value='"+newval+"'\n";
		superstar_db.set(starpath,newval);
	}
	else {
		content+="AUTHENTICATION MISMATCH";
		printf("  Authentication mismatch: write to '%s' not authorized by '%s'\n",
			starpath.c_str(), sentauth);
	}

	// New optional syntax: /superstar/path1?set=newval1&get=path2,path3,path4
	if (0<=mg_get_var(conn,"get",buf,NBUF))
	{
		std::string retArray="[";
		char *bufLoc=buf;
		while (0!=*bufLoc) {
			char *nextComma=strchr(bufLoc,','); // Find next comma
			if (nextComma!=0) *nextComma=0; // nul terminate at comma

			if (retArray.size()>1) retArray+=","; // add separator to output
			std::string value=superstar_db.get(bufLoc); // add path to output
			if (value=="") value="{}"; // mark empty JSON objects
			printf("   mget path: %s -> %s\n",bufLoc,value.c_str());
			retArray+=value;

			if (nextComma==0) break; // done with this path
			else bufLoc=nextComma+1; // move down string
		}
		retArray+="]";
		return send_json(conn,retArray);
	}
  }
  else { /* Not writing a new value */
  	if (query=="get")
  	{ /* getting raw JSON */
		return send_json(conn,superstar_db.get(starpath));
  	}
  	if (query=="sub") { /* substring search */
  		return send_json(conn,superstar_db.substrings(starpath));
  	}

  	// fixme: "after" type query (requires thread suspend)

  	std::string value=superstar_db.get(starpath);
  	if (value.size()>0)
		content+="<P>Current value: '"+value+"'";
	else { // no value set.  Substrings?
		std::string subs=superstar_db.substrings(starpath);
		std::string links=superstar_db.sublinks(starpath);

		if (subs.size()>2) // not just []
			content+="<P>Sub directories: "+links+"\n";
		else
			content+="<P>No data found.\n";
	}
  }


  content+="</BODY></HTML>";

  // Send human-readable HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            content.size(), content.c_str());

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return MG_TRUE;
}

/*
  This code runs a mongoose server in a dedicated thread.
  They're all listening on the same port, so the OS will
  hopefully distribute the load among threads.
*/
void *thread_code(void* data)
{
	struct mg_server *server= mg_create_server(NULL, http_handler);
	mg_set_option(server, "listening_port", ADDRESS.c_str());
	mg_set_option(server, "ssi_pattern", "**.html$");
	mg_set_option(server, "document_root", "../www"); // files served from here

	printf("OK, listening!  Visit http://localhost:%s to see the site!\n",
	mg_get_option(server, "listening_port"));

	while(true)
	{
		mg_poll_server(server,1000);

		if(data!=NULL)
		{
			int64_t new_time=millis();

			if((new_time-old_time)>=backup_time)
			{
				try
				{
					superstar_db.save(backup_filename);
					// std::cout<<"Saved backup file \""+backup_filename+"\"."<<std::endl;
				}
				catch(std::exception& error)
				{
					std::cout<<"Error saving superstar DB backup file: "<<error.what()<<std::endl;
				}

				old_time=new_time;
			}
		}
	}

	return 0;
}


int main(int argc, char *argv[])
{
	try
	{
		superstar_db.load(backup_filename);
		std::cout<<"Loaded backup file \""+backup_filename+"\"."<<std::endl;
	}
	catch(std::exception& error)
	{
		std::cout<<error.what()<<std::endl;
	}

	for (int argi = 1; argi < argc; argi++) {
		if (0 == strcmp(argv[argi], "--address")) ADDRESS = argv[++argi];
		else {
			printf("Unrecognized command line argument '%s'\n", argv[argi]);
		}
	}

	/* Start threads to be redundant servers.  This seems to do nothing. */
	for (int thread=0;thread<0;thread++)
		mg_start_thread(thread_code,NULL);

	thread_code((void*)1); // devote main thread to listening as well as saving
	return 0;
}
