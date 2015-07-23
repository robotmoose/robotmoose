# RobotMoose Web Robotics System

This is a simple, modern version of a networked robotics stack, built around the modern web.  The major pieces are:

* A web **front end** for user friendly robot setup, teleoperation, and programming.  See www/js for the code, which is built in JavaScript on bootstrap.
* A central JSON server called **superstar**, used by the robot to make its sensor values available to the front end (as /superstar/*robotname*/sensors), used the by the front end to post robot commands (at /superstar/*robotname*/pilot), and used for persistent storage as well.  See superstar/ for the code, which is built in C++ on Mongoose, but the HTTP get and set commands can be sent from any language, and is conceptually similar to MongoDB.
* An on-robot **back end**, that relays superstar commands to the robot hardware, and keeps track of the robot location.  See tabula_rasa/ for a C++ version.
* A runtime configurable Arduino **firmware**, which allows new robot hardware to be added at runtime without re-flashing or editing the code.

The typical usage model puts the front end on the robot pilot's web browser, superstar on a cloud server accessible from anywhere, the backend on a laptop onboard the robot, and the Arduino directly wired into the robot hardware.  But you can run all the software on a single low-end laptop for a network-free version, or use a dedicated superstar on a closed network, or many other permutations.

## Installation and Setup

First, you need the version control system git.  For Windows, use [Git for Windows](https://msysgit.github.io/).  For Ubuntu, use:
```
sudo apt-get install git make g++ freeglut3-dev arduino
```

Now clone our repository, and rename it "robotmoose":
```
git clone https://github.com/robotmoose/main
mv main ~/robotmoose
```

The easy way to get started is to connect a **simulated** robot backend to the default robotmoose.com superstar server:
```
cd ~/robotmoose/tabula_rasa
make
./backend --robot yourbot --sim
```

You should immediately be able to pilot the simulated robot by entering the robot name of "yourbot" at http://robotmoose.com/pilot/

### Firmware Installation and Physical Backend
To use a real physical Arduino with real robot components, first plug in the Arduino and flash the firmware at robotmoose/tabula_rasa/arduino, using either the Arduino IDE or the Makefile there:
```
cd ~/robotmoose/tabula_rasa/arduino
make
```
You can doublecheck the installation by opening the Arduino as a serial port at 57600 baud, and you should get a welcome message.

Now run the backend again, but leave off the "--sim" argument, and the backend will connect to the Arduino and configure it:
```
cd ~/robotmoose/tabula_rasa
make
./backend --robot yourbot
```

### Making a Superstar 

To edit the web front end locally, you need to change the files in robotmoose/www/js, and serve them using your own local copy of superstar:
```
cd ~/robotmoose/superstar
make
./superstar > log &
```

You should now be able to point your web browser to http://localhost:8081/pilot/ and see your local copy of the web user interface.  Connect a robot backend to this local interface using:
```
cd ~/robotmoose/tabula_rasa
make
./backend --robot yourbot --superstar http://localhost:8081/
```

### Fun Tricks
The superstar server has been tested with up to ten robots and pilots connected.  Normally, each robot has a unique name, which allows the pilot to control it.

If you give several robots the same name, the pilot interface jumps between their sensor data as they report it, which isn't very useful.  But all the robots read the same pilot commands, so they move together like synchronized swimmers!

If several pilots connect to the same robot, they all see the same sensor data, but the pilot's commands overwrite each other, which makes for very jerky driving.



## Project Background

This is an NSF-funded ITEST research project, with full name:
Collaborative Research: ITEST-Strategies: Human-Centered Robotics Experiences for Exploring Engineering, Computer Science, and Society

Seta Bogosyan <sbogosyan@alaska.edu>, PI
Orion Lawlor <lawlor@alaska.edu>, Co-PI
Cindy Hmelo-Silver <chmelosi@gmail.com>, Co-PI
Selma Sabanovic <selmas@indiana.edu>, Co-PI



To get all the backend scripts on an Ubuntu linux machine, including Raspbian:
	sudo apt-get install freeglut3-dev cheese mpg321 imagemagick

You'll also need a video conferencing (WebRTC) capable browser:
	sudo apt-get install iceweasel


