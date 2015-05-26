Server is a "forwarder" that runs on port 8080. This should be hosted on the main server that would host many different web cameras.

Client is actually a server, but it is a client to the forwarder, that runs on 8081. The ip address of the client needs to be in the cams list in server.cpp.

You can run a client without a server, but this is a single webcam server. The forwarder allows one to type in the ip address of the client into the forwarder, and view images from that client.
