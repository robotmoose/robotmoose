/**
  Superstar: the "star" in the center of our network topology.
  This is an HTTP server on a public IP, used to connect the pilot
  and robot, who both may live behind firewalls.
  
  
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-10-02 (Public Domain)
*/
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include "mongoose/mongoose.h"

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
	
	/**
	  Overwrite the current value in the database with this new value.
	*/
	void set(const std::string &path,const std::string &new_value) {
		db[path]=new_value;
	}
	
	/**
	  Read the latest value from the database.
	*/
	const std::string &get(const std::string &path) {
		return db[path];
	}
	
	/**
	  Return a comma-separated JSON array of quoted substrings 
	  in the database matching this prefix. 
	*/
	std::string substrings(const std::string &path_prefix) {
		std::string list="";
		std::string last="";
		for (db_t::const_iterator c=db.begin();c!=db.end();++c) {
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
};

superstar_db_t superstar_db;


// This function will be called by mongoose on every new request.
int http_handler(struct mg_connection *conn, enum mg_event ev) {
  if (ev==MG_AUTH) return true; // everybody's authorized
  if (ev!=MG_REQUEST) return false; // not a request? not our problem
  // else it's a request
  
  printf("Incoming request: client %s:%d, URI %s\n",conn->remote_ip,conn->remote_port,conn->uri);
  
  const char *prefix="/superstar/";
  if (strncmp(conn->uri,prefix,strlen(prefix))!=0) 
  	return MG_FALSE; // file fallback
  std::string starpath(&conn->uri[strlen(prefix)]);

  std::string query="";
  if (conn->query_string) query=conn->query_string;
  
  std::string content="<HTML><BODY>Hello from mongoose!  "
  	"I see you're using source IP "+std::string(conn->remote_ip)+" and port "+my_itos(conn->remote_port)+"\n";
  content+="<P>Superstar path: ";
    content+=starpath;
  
  enum {NBUF=8192}; // maximum length for JSON data being set
  char buf[NBUF];
  if (0<=mg_get_var(conn,"set",buf,NBUF)) { /* writing new value */
  	std::string newval(buf);
	content+="<P>Setting new value='"+newval+"'\n";
	superstar_db.set(starpath,newval);
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
		if (subs.size()>2) // not just []
			content+="<P>Sub directories: "+subs+"\n";
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
void *thread_code(void *) {
  struct mg_server *server= mg_create_server(NULL, http_handler);
  mg_set_option(server, "listening_port", "8080");
  mg_set_option(server, "document_root", "web"); // files served from here

  printf("OK, listening!  Visit http://localhost:%s to see the site!\n",
  	mg_get_option(server, "listening_port"));
  
  while (1) {
  	mg_poll_server(server,1000);
  }
  return 0;
}


int main(void) {
  /* Start threads to be redundant servers.  This seems to do nothing. */
  for (int thread=0;thread<0;thread++)
  {
	mg_start_thread(thread_code,0);
  }
  thread_code(0); // devote main thread to listening as well
  return 0;
}
