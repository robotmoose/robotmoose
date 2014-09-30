/**
  Demo Mongoose server, based on mongoose example "hello.c".
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-09-30 (Public Domain)
*/
#include <stdio.h>
#include <string.h>
#include <string>
#include "mongoose.h"

std::string my_itos(int i) {
	char buf[100];
	snprintf(buf,sizeof(buf),"%d",i);
	return buf;
}

// This function will be called by mongoose on every new request.
int http_handler(struct mg_connection *conn, enum mg_event ev) {
  if (ev==MG_AUTH) return true; // everybody's authorized
  if (ev!=MG_REQUEST) return false; // unknown request
  // else it's a request
  
  printf("Incoming request: client %s:%d, URI %s\n",conn->remote_ip,conn->remote_port,conn->uri);
  
  const char *prefix="/superstar";
  if (strncmp(conn->uri,prefix,strlen(prefix))!=0) 
  	return MG_FALSE; // file fallback?

  std::string content="<HTML><BODY>Hello from mongoose!  "
  	"I see you're using source IP "+std::string(conn->remote_ip)+" and port "+my_itos(conn->remote_port)+"\n"
  	"</BODY></HTML>";

  // Send HTTP reply to the client
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

int main(void) {
  struct mg_server *server= mg_create_server(NULL, http_handler);
  mg_set_option(server, "listening_port", "8080");
  mg_set_option(server, "document_root", "."); // files served from here
  
  // Wait until user hits "enter". Server is running in separate thread.
  printf("OK, listening!  Visit http://localhost:%s to see the site!\n",
  	mg_get_option(server, "listening_port"));
  
  while (1) {
  	mg_poll_server(server,1000);
  }

  return 0;
}
