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
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdarg.h>
#include <stdlib.h>
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

static struct mg_serve_http_opts s_http_server_opts;



inline std::string mg_str_to_std_string(const mg_str *str)
{
	return std::string(str->p,str->len);
}

//Replace all instances of "find" with "replace" in "str".
std::string replace_all(std::string str,const std::string& find,const std::string& replace)
{
	size_t pos=0;

	while((pos=str.find(find,pos))!=std::string::npos)
	{
		str.replace(pos,find.size(),replace);
		pos+=replace.size();
	}

	return str;
}

std::string strip_slashes_start(std::string str)
{
	while(str.size()>0&&str[0]=='/')
		str=str.substr(1,str.size());
	return str;
}

std::string strip_slashes_end(std::string str)
{
	while(str.size()>0&&str[str.size()-1]=='/')
		str=str.substr(0,str.size()-1);
	return str;
}

std::string strip_slashes(std::string str)
{
	return strip_slashes_end(strip_slashes_start(str));
}

std::string strip_toplevel(std::string starpath)
{
	starpath=strip_slashes_end(starpath);
	while(starpath.size()>0&&starpath[starpath.size()-1]!='/')
		starpath=starpath.substr(0,starpath.size()-1);
	return starpath;
}

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
  We're done with this connection.
*/
void connection_done(struct mg_connection *conn)
{
	// This uses up way too many ports on the server--
	//  leaving it out enables HTTP keepalive?
	// conn->flags |= MG_F_SEND_AND_CLOSE;
}

/**
  Simple mongoose utility function: send back raw JSON reply.
  This is designed to be minimal, and easy to parse in JavaScript or C++.
*/
void send_json(struct mg_connection *conn,std::string json)
{
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/json\r\n"
		"Content-Length: %ld\r\n"        // Always set Content-Length
		"\r\n"
		"%s",
		json.size(), json.c_str());

	connection_done(conn);
}

//  Same as send_json, but returns 401 Unauthorized.
void send_json_auth_error(struct mg_connection *conn,std::string json)
{
	mg_printf(conn,
		"HTTP/1.1 401 Unauthorized\r\n"
		"Content-Type: text/json\r\n"
		"Content-Length: %ld\r\n"        // Always set Content-Length
		"\r\n"
		"%s",
		json.size(), json.c_str());

	connection_done(conn);
}

/**
  This is the key-value "database" of everything stored by superstar.
  Eventually it would be nice to persist this to disk, and keep history, although
  that's probably not needed for robotics piloting, which tends to be same-day.
*/
class superstar_db_t {
private:

	class database_entry {
	public:
		// JSON object at this path
		std::string value;

		// Current client connections waiting for getnext data
		std::vector<struct mg_connection *> waiting_connections;

		database_entry(void) :value("") {}
		database_entry(const std::string &newval) :value(newval) {}
	};

	typedef std::map<std::string /* path */, database_entry> db_t;
	db_t db;


	// Normalize this path name--everything gets one trailing slash
	std::string path_normalize(std::string path) const {
		if(path.size()>0&&path[path.size()-1]!='/')
			path+="/";

		//if(path.size()==0)
		//	path+="/";

		while(path.size()>=2 && path[path.size()-1]=='/' && path[path.size()-2]=='/')
			path.erase(path.end()-1); // trim extra ending slash

		if(path[0]=='/') // trim weird leading slashes (front end bug?)
			path.erase(path.begin());

		return path;
	}
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

			uint64_t data_size=entry->second.value.size();
			std::string data_str=entry->second.value;
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
		path=path_normalize(path);
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
			database_entry &e=db[path];
			if (e.value!=new_value) {
				e.value=new_value;

				setnext(path,e); // any waiting getnext values
			}
		}
	}

	/**
	  Read the latest value from the database.
	*/
	const std::string& get(std::string path)
	{
		db_t::iterator iter=db.find(path_normalize(path));
		if (iter!=db.end()) {
			return (*iter).second.value; // i.e., db[path];
		}
		else {
			static const std::string empty="";
			return empty;
		}
	}

	/**
	  Get this path's value back to this connection,
	  after the next value is put in the database.
	*/
	void getnext(std::string path,struct mg_connection *conn)
	{
		database_entry &e=db[path_normalize(path)];
		e.waiting_connections.push_back(conn);
		conn->flags |= MG_F_USER_1; // mark as in use
		conn->user_data=(void *)&e;
	}

	// Called by set after the value changes:
	void setnext(const std::string &path,database_entry &e)
	{
		// Send new value to all waiting clients:
		for (unsigned int i=0;i<e.waiting_connections.size();i++) {
			struct mg_connection *conn=e.waiting_connections[i];
			conn->flags &= ~MG_F_USER_1; // mark as done
			conn->user_data=0; // clear pointer
			printf("  sent off new value to getnext client\n");
			send_json(conn,e.value);
		}
		// Remove them from the list
		e.waiting_connections.clear();
	}

	// This connection is shutting down--cancel its getnext
	bool connection_closing(struct mg_connection *conn) {
		if ((conn->flags & MG_F_USER_1) && conn->user_data) { // connection is getnext still in progress
			printf("Closing an in-progress getnext (WEIRD CASE): ");

			database_entry &e=*(database_entry *)conn->user_data;

			// Check if this is the doomed connection
			//  (This avoids future sends to the doomed one,
			//   which would yield a use-after-delete memory corruption)
			for (unsigned int i=0;i<e.waiting_connections.size();i++) {
				if (e.waiting_connections[i]==conn) {
					e.waiting_connections.erase(e.waiting_connections.begin()+i);
					printf("found and erased it.  OK.\n");
					return true;
				}
			}
			// No sign of that connection...
			printf("no sign of closing getnext (SCARY!)\n");
			return false;
		}

		// Fine--not a getnext in progress
		return true;
	}

	/**
	  Return a comma-separated JSON array of quoted substrings
	  in the database matching this prefix.
	*/
	std::string substrings(std::string path_prefix)
	{
		path_prefix=path_normalize(path_prefix);

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
		path_prefix=path_normalize(path_prefix);

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


//	Return true is auth was set succesfully.
// If false, then we had an error in file operations
bool set_auth(std::string starpath, std::string new_auth)
{
	if(starpath[0]!='/')
		starpath="/" + starpath;
	std::ofstream f("auth",std::ios_base::app);
	if(f.is_open())
	{
		f<<starpath+"\t"+new_auth;
		f<<std::endl;
		f.close();
		return true;
	}	
	else 
		return false;
	


}

/**
  Return true if a write to this starpath is allowed.
  The variable orig_starpath refers to the original starpath passed,
	since this is a recursive function, the starpath variable gets the
	top level stripped off to check for higher privileged auth codes.
*/
bool write_is_authorized(std::string starpath,
	const std::string &new_data,const std::string &new_auth,
	std::string orig_starpath="")
{
	if(orig_starpath.size()==0)
		orig_starpath=starpath;

	//Try to open auth file
	std::ifstream f("auth");

	//No passwords at all, return true
	if(!f.good())
		return true;

	//Open auth file and parse passwords in line based format "PATH PASSWORD" (without quotes)
	bool found=false;
	std::string pass;
	while(std::getline(f,pass))
	{
		std::istringstream istr(pass);
		std::string path;
		istr>>path;
		//Strip beginning and ending slashes to make passwords easier...
		if(strip_slashes(path)==strip_slashes(starpath))
		{
			//Empty password...
			if(!(istr>>pass))
				pass="";
			found=true;
			break;
		}
	}

	//No password found
	if(!found)
	{
		//Remove top level of path
		std::string new_starpath=strip_toplevel(starpath);

		//If top level path isn't blank and not the same as the old one, try it's auth
		if(new_starpath!=starpath)
			return write_is_authorized(new_starpath,new_data,new_auth,orig_starpath);

		//Else no auth, writeable
		return true;
	}

	//Blank password is the same as no password
	if(pass.size()<=0)
		return true;

	//If we get here, there is a password.  Verify there is an authentication code.
	if (new_auth.size()<=0)
		return false; // need authentication code

	//FIXME: extract sequence number(read JSON in new_data?)
	int seq=0;
	std::string alldata=pass+":"+orig_starpath+":"+new_data+":"+my_itos(seq);

	//Check to see what auth code should be
	std::string should_auth=getAuthCode<SHA256>(alldata);

	//If auths don't match, parent's auth works as well, so check that
	if(should_auth!=new_auth)
	{
		//Remove top level of path
		std::string new_starpath=strip_toplevel(starpath);

		//If top level path isn't blank and not the same as the old one, try it's auth
		if(new_starpath!=starpath)
			return write_is_authorized(new_starpath,new_data,new_auth,orig_starpath);
	}

	//Otherwise, return if the auth is what it should be
	return (should_auth==new_auth);
}

// This function will be called by mongoose on every new request.
void superstar_http_handler(struct mg_connection *conn, int ev,void *param) {
	bool auth_error = false;

	if (ev==MG_EV_CLOSE) {
		superstar_db.connection_closing(conn);
	}
  if (ev!=MG_EV_HTTP_REQUEST) return; // not a request? not our problem
  // else it's an http request
  struct http_message *m=(struct http_message *)param;

  std::string remote_ip('?',100);
  mg_sock_addr_to_str(&conn->sa,&remote_ip[0],remote_ip.size(),MG_SOCK_STRINGIFY_IP);
  std::string remote_port('?',100);
  mg_sock_addr_to_str(&conn->sa,&remote_port[0],remote_port.size(),MG_SOCK_STRINGIFY_PORT);

  if(remote_ip=="127.0.0.1")
  {
		// std::cout<<"Local IP detected, attempting to get remote..."<<std::flush;

		for(int ii=0;ii<MG_MAX_HTTP_HEADERS;++ii)
		{
			if(mg_str_to_std_string(&m->header_names[ii])=="X-Forwarded-For")
			{
				remote_ip=mg_str_to_std_string(&m->header_values[ii]);
				// std::cout<<"success, X-Forwarded-For header found."<<std::endl;
				break;
			}
		}

		// if(remote_ip=="127.0.0.1")
		//	std::cout<<"failed."<<std::endl;
  }

  printf("Incoming request: client %s:%s, URI %s\n",remote_ip.c_str(),remote_port.c_str(),mg_str_to_std_string(&m->uri).c_str());

  const char *prefix="/superstar/";
  if (strncmp(m->uri.p,prefix,strlen(prefix))!=0)
  { // not superstar--serve raw files instead
	mg_serve_http(conn, m, s_http_server_opts);
	return;
  }

  std::string starpath=mg_str_to_std_string(&m->uri).substr(strlen(prefix));

  //Hacky way to replace multiple slashes with a single slash
  for(size_t ii=0;ii<starpath.size()/2+1;++ii)
	starpath=replace_all(starpath,"//","/");

  std::string query=mg_str_to_std_string(&m->query_string);

  std::string content="<HTML><BODY>Hello from mongoose!  ";

//  "I see you're using source IP "+remote_ip+" and port "+my_itos(conn->remote_port)+"\n";
  content+="<P>Superstar path: "+starpath+"\n";

	enum {NBUF=32767}; // maximum length for JSON data being set
	char buf[NBUF];




	//appending and/or trimming value
	if(0<=mg_get_http_var(&m->query_string,"append",buf,NBUF)||0<=mg_get_http_var(&m->query_string,"trim",buf,NBUF))
	{
		//appending value
		if(0<=mg_get_http_var(&m->query_string,"append",buf,NBUF))
		{
			std::string appendval(buf);
			char sentauth[NBUF];

			//empty authentication string
			if(0>mg_get_http_var(&m->query_string,"auth",sentauth,NBUF))
				sentauth[0]=0;

			if(write_is_authorized(starpath,appendval,sentauth))
			{
				std::string old_value=superstar_db.get(starpath);
				std::string new_value=old_value;
				if(new_value.size()>0)
					new_value+="\n";
				new_value+=appendval;
				content+="<P>Appended new value='"+new_value+"'\n";
				superstar_db.set(starpath,new_value);
			}
			else
			{
				auth_error = true;
				content+="AUTHENTICATION MISMATCH";
				printf("  Authentication mismatch: append to '%s' not authorized by '%s'\n",
					starpath.c_str(),sentauth);
			}
		}

		//trimming value
		if(0<=mg_get_http_var(&m->query_string,"trim",buf,NBUF))
		{
			std::string trimval(buf);
			char sentauth[NBUF];

			//empty authentication string
			if(0>mg_get_http_var(&m->query_string,"auth",sentauth,NBUF))
				sentauth[0]=0;

			if(write_is_authorized(starpath,trimval,sentauth))
			{
				std::istringstream istr(trimval);
				int wanted_lines;
				if(istr>>wanted_lines)
				{
					int actual_lines=1;
					std::string new_value=superstar_db.get(starpath);
					for(size_t ii=0;ii<new_value.size();++ii)
						if(new_value[ii]=='\n')
							++actual_lines;

					for(int ii=0;ii<actual_lines-wanted_lines;++ii)
					{
						while(new_value.size()>0&&new_value[0]!='\n')
							new_value=new_value.substr(1,new_value.size()-1);
						if(new_value.size()>0)
							new_value=new_value.substr(1,new_value.size()-1);
					}
					superstar_db.set(starpath,new_value);
					content+="<P>Trimmed new value='"+new_value+"'\n";
				}
			}
			else
			{
				auth_error = true;
				content+="AUTHENTICATION MISMATCH";
				printf("  Authentication mismatch: trim to '%s' not authorized by '%s'\n",
					starpath.c_str(),sentauth);
			}
		}
	}

	//writing new value
	else if(0<=mg_get_http_var(&m->query_string,"set",buf,NBUF))
	{
		std::string newval(buf);
		char sentauth[NBUF];

		//empty authentication string
		if(0>mg_get_http_var(&m->query_string,"auth",sentauth,NBUF))
			sentauth[0]=0;

		if(write_is_authorized(starpath,newval,sentauth))
		{
			content+="<P>Setting new value='"+newval+"'\n";
			superstar_db.set(starpath,newval);
		}
		else
		{
			auth_error = true;
			content+="AUTHENTICATION MISMATCH";
			printf("  Authentication mismatch: write to '%s' not authorized by '%s'\n",
				starpath.c_str(), sentauth);
		}
	}


	else if (0<=mg_get_http_var(&m->query_string,"setauth",buf,NBUF))
	{
		std::string new_auth(buf);
		char sentauth[NBUF];

		if(0>mg_get_http_var(&m->query_string,"auth",sentauth,NBUF))
			sentauth[0]=0;
		if(write_is_authorized(starpath,new_auth,sentauth))
		{
			content+="<P>Setting new auth+\n";
			bool set_successfull=set_auth(starpath,new_auth);
			if(set_successfull)
				content+="<P> Auth set succesfully";
			else
			{
				content+="<P> Failed to set auth. Check if you have permission";
			}
		}
		else
		{	
			content+="<P> Failed to set auth. Do you have permission?";	
			auth_error=true;
		}
	}



	//Not writing a new value
	else
	{
		if (0<=mg_get_http_var(&m->query_string,"getnext",buf,NBUF))
		{ /* getting JSON when it next changes (COMET / Hanging Get / "Get on Set")
			 from the given value.  Send current value to avoid update race. */
			if (buf!=superstar_db.get(starpath))
			{ // need new value immediately
				printf("Sending getnext client immediate data\n");
				send_json(conn,superstar_db.get(starpath));
			}
			else
			{ // wait for new value to arrive
				printf("Queuing up the getnext client\n");
				conn->flags |= MG_F_USER_1; // mark as getnext in progress
				superstar_db.getnext(starpath,conn);
			}
			return;
		}
		if (query=="get")
		{ /* getting raw JSON */
			send_json(conn,superstar_db.get(starpath));
			return;
		}
		if (query=="sub") { /* substring search */
			send_json(conn,superstar_db.substrings(starpath));
			return;
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

	//New optional syntax: /superstar/path1?set=newval1&get=path2,path3,path4
	if(0<=mg_get_http_var(&m->query_string,"get",buf,NBUF))
	{
		std::string retArray="[";
		char* bufLoc=buf;
		while(0!=*bufLoc)
		{
			//find next comma
			char* nextComma=strchr(bufLoc,',');

			//null terminate at comma
			if(nextComma!=0)
				*nextComma=0;

			//add separator to output
			if(retArray.size()>1)
				retArray+=",";

			//add path to output
			std::string value=superstar_db.get(bufLoc);

			// mark empty JSON objects
			if(value=="")
				value="{}";

			printf("   mget path: %s -> %s\n",bufLoc,value.c_str());
			retArray+=value;

			//done with this path
			if(nextComma==0)
				break;

			// move down string
			else
				bufLoc=nextComma+1;
		}
		retArray+="]";
		if (auth_error) {
			send_json_auth_error(conn, retArray);
		} else {
			send_json(conn,retArray);
		}
		return;
	}


  content+="</BODY></HTML>";

  // Send human-readable HTTP reply to the client
	if (auth_error) {
		mg_printf(conn,
				"HTTP/1.1 401 Unauthorized\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %ld\r\n"        // Always set Content-Length
				"\r\n"
				"%s",
				content.size(), content.c_str());
	} else {
		mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: %ld\r\n"        // Always set Content-Length
				"\r\n"
				"%s",
				content.size(), content.c_str());
	}

  // We're done--close once data has been sent
  connection_done(conn);
}

/*
  This code runs a mongoose server in a dedicated thread.
  They're all listening on the same port, so the OS will
  hopefully distribute the load among threads.
*/
void *thread_code(void* data)
{
	struct mg_mgr mgr;
	struct mg_connection *nc;

	mg_mgr_init(&mgr, NULL);
	nc = mg_bind(&mgr, ADDRESS.c_str(), superstar_http_handler);

	if (nc==NULL)
	{
		std::cout<<"Could not open port "<<ADDRESS<<"."<<std::endl;
		std::cout<<"\tDo you have permission?"<<std::endl;
		std::cout<<"\tIs something already running on that port?"<<std::endl;
		exit(1);
	}

	// Set up HTTP server parameters:
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = "../www";  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";
	s_http_server_opts.ssi_pattern="**.html$";

	while(true)
	{
		mg_mgr_poll(&mgr, 1000);

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
	mg_mgr_free(&mgr);

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

	/* Start threads to be redundant servers.  This seems to do nothing.
	for (int thread=0;thread<0;thread++)
		mg_start_thread(thread_code,NULL);
	*/

	thread_code((void*)1); // devote main thread to listening as well as saving
	return 0;
}
