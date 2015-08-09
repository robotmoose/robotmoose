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
./backend --robot test/yourbot --sim
```

You should immediately be able to pilot the simulated robot by choosing the robot at http://robotmoose.com/pilot/


### Firmware Installation and Physical Backend
To use a real physical Arduino with real robot components, you first need to make sure you have permission to access the USB serial port where the Arduino connects.  On Ubuntu, you need to be a member of the "dialout" group:
```
sudo usermod -a -G dialout $USER
```
You'll need to log out and log back in to be in the new group.

Now plug in the Arduino and flash the firmware at robotmoose/tabula_rasa/arduino, using either the Arduino IDE or the Makefile there:
```
cd ~/robotmoose/tabula_rasa/arduino
make
```
You can doublecheck the installation by opening the Arduino as a serial port at 57600 baud, and you should get a welcome message.

Now run the backend again, but leave off the "--sim" argument, and the backend will connect to the Arduino and configure it:
```
cd ~/robotmoose/tabula_rasa
make
./backend --robot test/yourbot
```

### Configure and Pilot your Robot

You access the main robot web interface at [http://localhost:8081/pilot/](http://localhost:8081/pilot/).  
* Connect to the school ("test") and robot name ("yourbot") you gave the backend above.
* Configure the robot in the "Configure" tab by selecting a piece of hardware and hitting "Add".  I always start with a "heartbeat" so I can verify communication works.  Hit "Configure" to copy this configuration to the Arduino.
* Check the "Sensors" tab to see the data sent back by your robot.  The "heartbeat" is a number that counts up from 0 to 255 again and again, so you can see the Arduino is connected.  The "location" is a guess of the position of the robot, and is updated as the robot moves.

There are many types of robot driving hardware (tabula_rasa/arduino/motor_controller.cpp) supported by this system.  Once you configure any of these robot motors, the "Drive" tab will gain a set of arrows allowing you to drive the robot!
* "create2" is a serial connection to an iRobot Create 2 or 500/600 series Roomba.  When you press "Configure", you need to press the Roomba's power button to wake up the Roomba, or the Arduino will sit waiting for the Roomba to connect.
* "bts" represents two BTS 7960B motor controllers, used to drive the left and right wheels of your robot.  There are four pins listed, two PWM pins for each motor.  (Tie the BTS's VCC and EN lines to Arduino 5V, and the GND line to Arduino ground.  You can leave the IS pins unconnected.)
* "sabertooth2" is a Dimension Engineering SaberTooth dual motor controller, version 2.  You set the serial port that sends serial commands to this device.  (Set address 128, packetized serial format, 9600 baud)
* "sabertooth1" is a Dimension Engineering SaberTooth, version 1.  (Set simple serial format, 9600 baud)

There are also several peripherals supported:
* "analog" reads an Arduino analog pin, like A3.  The read value will show up in the Sensors tab.
* "servo" is an RC servo, like with a model plane.  You can hook the servo's white control wire to any Arduino pin, and power the red from 5v and black to ground.  In the "Drive" tab, you set the servo position in degrees, from 0 to 180 degrees. 
* "pwm" is a pulse width modulated (PWM) pin, which can be used to smoothly vary the brightness of an LED.  For example, pin 13 is the standard Arduino LED, so you can control it by adding a pwm device on pin 13, and dragging the brightness slider around.
* "neato" is a Neato XV-11 laser distance sensor, which uses a laser to measure distance in a 360 degree plane around the robot.  You give a serial port to read the neato's serial data distance reports, and a pin to control the neato's motor power via a transistor.
* "latency" measures the Arduino firmware's control loop latency, in milliseconds.  It should normally read 1 millisecond.
* "heartbeat" is incremented every 10ms by Arduino firmware.  If you can see this number changing in the "Sensors" tab, you have connectivity between the web front end, superstar, backend, and Arduino.  If this number stops changing, something has been disconnected.



### Making a Superstar 

To edit the web front end locally, you need to change the files in robotmoose/www/js, and serve them using your own local copy of superstar:
```
cd ~/robotmoose/superstar
make
./superstar > log &
```

Connect a robot backend to this local interface using:
```
cd ~/robotmoose/tabula_rasa
make
./backend --robot test/yourbot --superstar http://localhost:8081/
```

You should now be able to point your web browser to [http://localhost:8081/pilot/](http://localhost:8081/pilot/) and see your local copy of the web user interface.  Edit the files in ~/robotmoose/www/js and hit reload to modify the interface.


### Fun Tricks
The superstar server has been tested with up to ten robots and pilots connected.  Normally, each robot has a unique name, which allows the pilot to control it.

If you give several robots the same name, the pilot interface jumps between their sensor data as they report it, which isn't very useful.  But all the robots read the same pilot commands, so they move together like synchronized swimmers!

If several pilots connect to the same robot, they all see the same sensor data, but the pilot's commands overwrite each other, which makes for very jerky driving.



## Debugging
The real trick with debugging is slowly and systematically looking at what is happening.

### Arduino permission error
If you have an Arduino permission problem, flashing the Arduino from the command line (cd ~/robotmoose/tabula_rasa/arduino; make) will error with:
```
avrdude: ser_open(): can't open device "/dev/ttyACM0": Permission denied
ioctl("TIOCMGET"): Invalid argument
ioctl("TIOCMGET"): Invalid argument
```

Even if you manage to flash the Arduino, if you have a permission problem the backend will hang when connecting to the Arduino:
```
Uploading new config to arduino!
Arduino startup: 
```

The fix for both problems is to make the Arduino device (/dev/ttyACM0 here, sometimes /dev/ttyUSB0) readable and writeable by your user account:
```
sudo usermod -a -G dialout $USER
```
You'll need to log out and log back in to be in the new group, and then the Arduino should work correctly.




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


