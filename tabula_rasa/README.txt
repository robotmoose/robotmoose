The idea here is a fully dynamically configurable Arduino firmware.

For example, to use a pair of BTS motor controllers on pins 8,9,10, and 11,
	./backend --robot "tester"  --motor "bts(8,9,10,11);"

You can then go to http://robotmoose.com/pilot/ and enter a Robot Name of "tester" to drive these pins.



Inside the firmware:
"action" is used for scheduling object polling.
    For example:  actions_10ms.add(new mySensor);
    (mySensor inherits from action)

"tabula_control" is used to get commands and send sensor values from a single shared buffer 
(FIXME: it should be possible to access these buffers via an efficient binary protocol)
    For example:  tabula_command_value<unsigned char> pwm;
    The command is stored in the command buffer at pwm.get_index();
    The currently commanded value is accessible via pwm.get();

"tabula_config" is used to build the above objects at runtime.
    You register your classes using REGISTER_TABULA_DEVICE,
    giving the setup name and some code to run when the device is created.
    For example, you can connect over serial port and enter:
	analog_sensor A3   (Create an analog input device on pin A3)
	sensors16         (Starts showing sensor values as 16-bit numbers)
	pwm_pin 3         (Create a PWM output device on pin 3)
	cmd 0 200         (Set command index 0 to value 200)
	ramp 0            (Change the PWM command index continuously)
	cmd 1 10          (Set rate of change of PWM command)
	loop!             (Done with configuration, enter main loop())



Bring up Create 2, on a Mega's TX3/RX3 lines:
	- Create 2 top light MUST be green.  Press clean or CLOCK until this is true.
	- Start backend and configure create2_controller_t.  
	- Top light MUST turn red, or it hasn't started properly.
	- Press clean again to power off so it can charge (forget this, and the battery will slowly die!)


Old Spring 2015 instructions:
reset!
create2_controller_t X3
sensors16

Create 2 pinout, viewed from vacuum cleaner side looking toward bumpers
 TX3      RX3

     gnd 


Bring up Mike's Leila:
reset!
commands16
bts_controller_t 3 5 6 9
pwm_pin A2
cmd 4 255
pwm_pin A3
cmd 5 255
pwm_pin A4
cmd 6 255
pwm_pin A5
cmd 7 255

Driving:
cmd16 0 50 cmd16 2 50
cmd16 0 0 cmd16 2 0



