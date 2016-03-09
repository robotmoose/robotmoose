Descriptions
	-Forwarder runs on port 8080. This should be hosted on the main server that would host many different web cameras.
	-Server runs on 8081. This is a webcam server.

Standard Operating Procedure (using forwarder):
	-Start a server on the needed robotic systems.
	-Add their ip addresses to the cams map in forwarder.cpp and recompile forwarder (not the best interface...but this is how it is currently...).
	-Start up the forwarder, type in the ip address of the wanted robotic system in the forwarder web page.

Standard Operating Procedure (without using forwarder):
	-Start a server on a robotic system.
	-Goto the server's web page, you should see the stream from the webcam.
