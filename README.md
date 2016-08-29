# RobotMoose Web Robotics System

[![Join the chat at https://gitter.im/robotmoose/robotmoose](https://badges.gitter.im/robotmoose/robotmoose.svg)](https://gitter.im/robotmoose/robotmoose?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This is a simple, modern version of a networked robotics stack, built around the modern web.  The major pieces are:

* A web **front end** for user friendly robot setup, teleoperation, and programming.  See www/js for the code, which is built in JavaScript on bootstrap.
* A central JSON server called **superstar**, used by the robot to make its sensor values available to the front end (as /superstar/*robotname*/sensors), used the by the front end to post robot commands (at /superstar/*robotname*/robots), and used for persistent storage as well.  See superstar/ for the code, which is built in C++ on Mongoose, but the HTTP get and set commands can be sent from any language, and is conceptually similar to MongoDB.
* An on-robot **back end**, that relays superstar commands to the robot hardware, and keeps track of the robot location.  See tabula_rasa/chromeapp for an easy to use RobotMoose chrome app GUI; or tabula_rasa/ for a command line C++ backend.
* A runtime configurable Arduino **firmware**, which allows new robot hardware to be added at runtime without re-flashing or editing the code.

The typical usage model puts the front end on the robot pilot's web browser, superstar on a cloud server accessible from anywhere, the backend on a laptop onboard the robot, and the Arduino directly wired into the robot hardware.  But you can run all the software on a single low-end laptop for a network-free version, or use a dedicated superstar on a closed network, or many other permutations.

## Installation and Setup

To get started with a simulated robot, just visit [robotmoose.com/robots/](https://robotmoose.com/robots/), and click the "Simulate a Robot" button. This will let you get a general sense of how the website works.

### Firmware Installation and Physical Backend

You need Google Chrome to run the backend, the version control system git to get this code, and the Arduino IDE to flash the Arduino firmware.

Windows:
* [Google Chrome](https://www.google.com/chrome/browser/desktop/index.html)
* [Git for Windows](https://msysgit.github.io/)
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)

macOS:
* [Google Chrome](https://www.google.com/chrome/browser/desktop/index.html)
* [GitHub for Mac](https://desktop.github.com/)
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)

Ubuntu:
* [Google Chrome](https://www.google.com/chrome/browser/desktop/index.html)
* Terminal command: `sudo apt-get install git arduino`

Now open Terminal (macOS/Ubuntu) or Command Prompt (Windows), and run this command to download the code:

    git clone https://github.com/robotmoose/robotmoose

On Ubuntu, you'll need to run this command to give yourself access to serial devices:

    sudo usermod -a -G dialout $USER

You'll need to log out and log back in to be in the new group.

Now plug in the Arduino and open the Arduino IDE. Open the project at `robotmoose/arduino/arduino.ino`, and click the upload button.

You can double check the installation by opening the Arduino as a serial port at 57600 baud. You should get a welcome message.

Open Chrome, and [install the backend app](https://chrome.google.com/webstore/detail/robotmoose/ecadjbcmgjcekpogchpochjieaimpegf?hl=en). Open the backend app, enter your year/school/robot info, and click connect.

You can now use the web interface to configure and pilot your robot!

### Configure and Pilot your Robot with the Web Front End

You access the main robot web interface at [http://robotmoose.com/robots/](http://robotmoose.com/robots/).  
* Connect to the school ("test") and robot name ("yourbot") you gave the backend above.
* Configure the robot in the "Configure" tab by selecting a piece of hardware and hitting "Add".  I always start with a "heartbeat" so I can verify communication works.  Hit "Configure" to copy this configuration to the Arduino.
* Check the "Sensors" tab to see the data sent back by your robot.  The "heartbeat" is a number that counts up from 0 to 255 again and again, so you can see the Arduino is connected.  The "location" is a guess of the position of the robot, and is updated as the robot moves.

There are many options for robot driving hardware (tabula_rasa/arduino/motor_controller.cpp) supported by this system.  Once you configure any of these robot motors, the "Drive" tab will gain a set of arrows allowing you to drive the robot!
* "create2" is a serial connection to an iRobot Create 2 or 500/600 series Roomba.  Arduino Pin 2 should be wired into the serial port wakeup line (otherwise when you press "Configure", you may need to press the Roomba's power button to wake up the Roomba, or the Arduino will sit waiting for the Roomba to connect).
* "bts" represents two BTS 7960B motor controllers, used to drive the left and right wheels of your robot.  There are four pins listed, two PWM pins for each motor.  (Tie the BTS's VCC and EN lines to Arduino 5V, and the GND line to Arduino ground.  You can leave the IS pins unconnected.)
* "sabertooth2" is a Dimension Engineering SaberTooth dual motor controller, version 2.  You set the serial port that sends serial commands to this device.  (Set address 128, packetized serial format, 9600 baud)
* "sabertooth1" is a Dimension Engineering SaberTooth, version 1.  (Set simple serial format, 9600 baud)

There are also several robot peripherals supported:
* "pwm" is a pulse width modulated (PWM) pin, which can be used to smoothly vary the brightness of an LED.  For example, pin 13 is the standard Arduino LED, so you can control it by adding a pwm device on pin 13, and dragging the brightness slider around.  This is a good first project to try!
* "servo" is an RC servo, like with a model plane.  You can hook the servo's white control wire to any Arduino pin, and power the red from 5v and black to ground.  In the "Drive" tab, you set the servo position in degrees, from 0 to 180 degrees.
* "analog" reads an Arduino analog pin, like A3.  The read value will show up in the Sensors tab.
* "neato" is a Neato XV-11 laser distance sensor, which uses a laser to measure distance in a 360 degree plane around the robot.  You give a serial port to read the neato's serial data distance reports, and a pin to control the neato's motor power via a transistor.
* "latency" measures the Arduino firmware's control loop latency, in milliseconds.  It should normally read 1 millisecond.
* "heartbeat" is incremented every 10ms by Arduino firmware.  If you can see this number changing in the "Sensors" tab, you have connectivity between the web front end, superstar, backend, and Arduino.  If this number stops changing, something has been disconnected.

### Making a Superstar
For most tasks, it's best to use UAF's main superstar server (http://robotmoose.com/superstar/).  But if you don't have reliable internet access, or want to change our web user interface code, you need your own local superstar server. These instructions assume you're running macOS or Ubuntu, with basic dev tools installed.

To make and run your own local copy of superstar:
```
cd ~/robotmoose/superstarv2
make
./superstar > log &
```  

When you first start your superstar, you will not have any robots. You will need to create them by using the make_robot.py utility in [utils/superstarv2](utils/superstarv2).  

Connect a robot backend to this local interface by setting the first dropdown to "127.0.0.1:8081".

You should now be able to point your web browser to [http://localhost:8081/robots/](http://localhost:8081/robots/) and see your local copy of the web user interface.  Edit the files in ~/robotmoose/www/js and hit reload to modify the interface.  You can also manually examine the entries in [http://localhost:8081/superstar/](http://localhost:8081/superstar/) to see how the robot configuration, sensor, and pilot information is exchanged, using superstar get and set operations.

Using the entire robotmoose system on your local network, with robots on different machines accessing your local superstar, requires a few extra steps:
* First, you must note the local IP address of the machine running your local superstar (for example, "192.168.1.24").
* Next, you must add this IP address to the Chrome App:
    * First, open up [tabula_rasa/chromeapp/manifest.json](tabula_rasa/chromeapp/manifest.json). To the permissions array, add the IP address of your local superstar, making sure to add "http" and port "8081" (e.g., "http://192.168.1.24:8081"). Mind the commas in the array!
    * Then, in [tabula_rasa/chromeapp/js/src/name.js](tabula_rasa/chromeapp/js/src/name.js), search for the `update_superstars_m` function. In the `superstar_options` array, add the IP address and port number of your local superstar (e.g. "192.168.1.24:8081").
* Finally, you must load the modified chrome app onto all of the backend computers:
    * Copy the modified chromeapp folder [tabula_rasa/chromeapp](tabula_rasa/chromeapp) and its files to the backend computer(s).
    * Go to [chrome://extensions/](chrome://extensions/), click the developer mode checkbox. If you have already loaded the chrome app from the git before, you will have to remove it because of the changes to manifest.json.
    * Click "load unpacked extension", browse to the chromeapp folder, then click ok.
    * When you launch the chrome app, the IP of your local superstar should appear in the backend dropdown!


### Fun Tricks
The superstar server has been tested with up to ten robots and pilots connected.  Normally, each robot has a unique name, which allows the pilot to control it.

If you give several robots the same name, the pilot interface jumps between their sensor data as they report it, which isn't very useful.  But all the robots read the same pilot commands, so they move together like synchronized swimmers!

If several pilots connect to the same robot, they all see the same sensor data, but the pilot's commands overwrite each other, which makes for very jerky driving.



## Debugging
The real trick with debugging is slowly and systematically looking at what is happening.

### Problem: Arduino permission error
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

A temporary fix is to force the Arduino device to be readable and writeable, but this will need to be repeated every time you unplug the Arduino:
```
sudo chmod 777 /dev/tty[UA]*
```

The permanent fix for both problems is to make all serial devices readable and writeable by your user account:
```
sudo usermod -a -G dialout $USER
```
You'll need to log out and log back in to be in the new group, and then any Arduino should work correctly, permanently.


### Problem: Internet Connection Error
The backend needs a network connection to the superstar server, or you will get errors like this:
```
Connecting to superstar at http://robotmoose.com/
terminate called after throwing an instance of 'skt_error'
  what():  Invalid domain name: 'robotmoose.com'
```

Check that the wireless network is connected correctly.  Can you reach google from the backend laptop?


### Problem: Robot does not drive
Things to look at:
* Are you getting a heartbeat in the "Sensors" tab?  If no heartbeat is listed, configure one.  If the heartbeat is shown but not counting, the problem is either the backend or the Arduino.  Is the backend laptop running, and does it have internet?  Is the backend running, and does it display any error messages?  Is the Arduino plugged in?
* Is the Roomba running?  If the heartbeat is running but the "light" sensors are not changing, you may need to go to the "Configure" tab and hit Configure again to restart the Roomba.  Also check the Roomba battery in the "Sensor" tab; a Roomba with a low battery will not drive.

### Problem: Superstar compilation fails with "Missing dependency operator"
If you get an error similar to this when you try to `make` Superstar:

	make: "/usr/home/christopher/Code/robotmoose/superstar/Makefile" line 7: Missing dependency operator
	make: "/usr/home/christopher/Code/robotmoose/superstar/Makefile" line 9: Need an operator
	make: Fatal errors encountered -- cannot continue
	make: stopped in /usr/home/christopher/Code/robotmoose/superstar

The problem is that Superstar's `Makefile` uses GNU make-specific features. To solve the problem, install `gmake` on your system (if it's not installed already), then run `gmake` instead of `make`.


## Project Background

This is an NSF-funded ITEST research project, with full name:
Collaborative Research: ITEST-Strategies: Human-Centered Robotics Experiences for Exploring Engineering, Computer Science, and Society

* Seta Bogosyan <sbogosyan@alaska.edu>, PI
* Orion Lawlor <lawlor@alaska.edu>, Co-PI
* Cindy Hmelo-Silver <chmelosi@gmail.com>, Co-PI
* Selma Sabanovic <selmas@indiana.edu>, Co-PI
