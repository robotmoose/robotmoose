/**
 This file handles serial communication with the
 tabula_rasa Arduino firmware.

 Mike Moss & Orion Lawlor, 2015-12, Public Domain
*/

function connection_t(on_message,on_disconnect,on_connect,on_name_set,on_auth_error,serial_api)
{
	if(!serial_api)
		serial_api=chrome.serial;
	var _this=this;
	_this.config="";
	_this.status_message=on_message;
	_this.on_disconnect=on_disconnect;
	_this.on_connect=on_connect;
	_this.on_name_set=on_name_set;
	_this.on_auth_error=on_auth_error;
	_this.connection_invalid="yes, totally invalid";
	_this.show_debug_bytes=false; // low level serial comm debugging
	_this.max_command=15; // A-packet formatting
	_this.max_short_length=15;
	_this.robot=null;
	_this.auth="";
	_this.serial_delay_ms=50; // milliseconds to wait between sensors and commands (saves CPU, costs some latency though)

	// Are there other serial JS apis?  maybe node.js?
	_this.serial_api=serial_api;

	_this.reset();

	// Set up event listeners:
	_this.serial_api.onReceive.addListener(
		function(info) { _this.serial_callback_onReceive(info); }
	);
	_this.serial_api.onReceiveError.addListener(
		function(info) { _this.serial_callback_onReceiveError(info); }
	);
}

connection_t.prototype.destroy=function()
{
	this.gui_disconnect();
}


// Reset all state
connection_t.prototype.reset=function() {
	var _this=this;

	_this.power={}; // pilot's power commanded values to Arduino
	_this.sensors={}; // Arduino-reported sensor values
	_this.sensors.power={}; // Commanded values reflected back up to pilot

	superstar.kill_comets();

	// Localization
	_this.location=new vec3(0,0,0);
	_this.angle=0;

	_this.serial_startup=true;
	_this.sends_in_progress=0;
	_this.connection=_this.connection_invalid;
	_this.port_name="not connected yet";
	_this.arduino_options=[]; // firmware-supported devices
	_this.device_names=[]; // configured devices
	_this.last_config={counter:-1};

	// ASCII line reader state
	_this.read_line="";
	_this.read_line_callback=null;

	// Binary A-packet reader state
	_this.read_state=0;
	_this.read_index=0;
	_this.read_sumpay=0;
	_this.read_packet=null; // callback
	_this.read_packet_data={valid:0,command:-1,length:0,data:null};
	_this.read_packet_data.toString=function()
	{ // debug dump an A-packet to the screen
		var p=_this.read_packet_data;
		var str="command 0x";
		str+=p.command.toString(16);
		str+=" payload "+p.length+" bytes: '";
		for (var i=0;i<p.length;i++) {
			var c=p.data[i];
			if (c>=32 && c<128) // ASCII
				str+=String.fromCharCode(c);
			else // non-ASCII
				str+=" 0x"+c.toString(16)+" ";
		}
		str+="'";
		return str;
	}
}

// Internal error handling function
connection_t.prototype.bad=function(why_string) {
	var _this=this;
	_this.status_message("ERROR: "+why_string);
	console.log("Serial connection error: "+why_string);
	_this.reconnect();
}

// Reconnect to the Arduino
connection_t.prototype.reconnect=function()
{
	var _this=this;
	var port_name=_this.port_name;
	_this.disconnect_m(function() {
		_this.connect_m(port_name);
	} );
}

// Status check
connection_t.prototype.connected=function()
{
	var _this=this;
	return _this.connection!==_this.connection_invalid;
}

// Callback to add new robot name
connection_t.prototype.gui_robot=function(robot)
{
	var _this=this;
	this.robot=robot;
	this.robot.auth=this.auth;
	if (_this.connected()) _this.reconnect();
}

// Callback to set auth
connection_t.prototype.gui_auth=function(auth)
{
	this.auth = auth;
	if (this.robot) this.robot.auth=auth;
}

// Callback from GUI
connection_t.prototype.gui_connect=function(port_name)
{
	var _this=this;
	_this.connect_m(port_name, function() {
		if(_this.on_connect)
			_this.on_connect();

		_this.save();
	} );
}

// Callback from GUI
connection_t.prototype.gui_disconnect=function(port_name,done_callback)
{
	var _this=this;
	_this.disconnect_m(function() {
		if(_this.on_disconnect)
			_this.on_disconnect();

		if (done_callback) done_callback();
	} );
}

/************* Serial Connection to Arduino ******************/

// Connect to this device
connection_t.prototype.connect_m=function(port_name,done_callback)
{
	var _this=this;
	_this.reset();
	if (!_this.robot) { _this.status_message("Need a school before connecting"); return; }

	_this.port_name=port_name;
	_this.status_message("Connecting to "+port_name);
	var options={
		bitrate:57600,
		receiveTimeout:5*1000 // in ms (must be more than Arduino 1.7 second start delay)
	};
	if(port_name=="Sim")
	{
		_this.serial_api=new sim_serial_t();
	}
	else
	{
		_this.serial_api=chrome.serial;
	}
	_this.serial_api.connect(port_name, options,
		function(connectionInfo) {
			_this.connection=connectionInfo.connectionId;
			_this.status_message("Connected to "+port_name);

			_this.serial_api.flush(_this.connection, function() {
				_this.arduino_setup_start();
				if (done_callback) done_callback();
			} );
		}
	);
}

// Disconnect from this device
connection_t.prototype.disconnect_m=function(done_callback)
{
	var _this=this;
	if (_this.connected()) {
		var connection=_this.connection;
		var port_name=_this.port_name;
		_this.reset();
		_this.status_message("Disconnecting from "+port_name);
		_this.serial_api.disconnect(connection,
			function() {
				if (chrome.runtime.lastError)
					_this.status_message("Error disconnecting from "+port_name);
				_this.status_message("Disconnected from "+port_name);

				if (done_callback) done_callback();
			}
		);
	}
}



connection_t.prototype.arduino_setup_start=function() {
	var _this=this;
	_this.serial_read_line( function(first_line) {
		_this.status_message(" Arduino boot message: "+first_line);
		_this.arduino_check_version();
	} );
}

// Check the firmware version number (during ASCII setup phase)
connection_t.prototype.arduino_check_version=function()
{
	var _this=this;
	_this.serial_send_ascii("version?\n", function() {
		_this.serial_read_line( function(line) {
			_this.status_message(" Got version line: "+line);
			_this.arduino_read_options();
		} );
	} );
}

// Read the list of configurable devices from Arduino (during ASCII setup phase)
connection_t.prototype.arduino_read_options=function()
{
	var _this=this;
	_this.serial_startup=false;
	_this.serial_send_ascii("list\n", function() {
		_this.serial_read_line( function(count_line) {
			var count=parseInt(count_line);
			if (count<1 || count>1000) _this.bad("Invalid device count "+count+" from line "+count_line);
			var handle_option=function(option_name) {
				_this.status_message(" Got option: "+option_name);
				if (option_name!="serial_controller ")
					_this.arduino_options.push(option_name);

				count--;
				if (count>0) _this.serial_read_line(handle_option);
				else _this.arduino_send_options();
			};
			_this.serial_read_line(handle_option);
		});
	});
}


// Upload device options to superstar
connection_t.prototype.arduino_send_options=function()
{
	var _this=this;
	_this.status_message(" Sending option list to superstar...");
	superstar_set(_this.robot,"options",_this.arduino_options,
		function()
		{
			_this.status_message(" Sent option list to superstar");
			_this.arduino_setup_devices();
		},
	function(err)
	{
		if(err.code==-32000)
		{
			var err="Authentication error connecting to Superstar!\nMake sure your password is correct.";
			_this.status_message(err);
			_this.gui_disconnect();
			if(_this.on_auth_error)
				_this.on_auth_error(err);
		}
		else
		{
			_this.status_message("Error connecting to Superstar: " + err);
		}
	}
	);
}

// Configure all devices on the Arduino
connection_t.prototype.arduino_setup_devices=function()
{
	var _this=this;

	var devices=["heartbeat();"];
	/*
		//"create2(X3);",
		"analog(A0);","analog(A2);", "analog(A5);",
		"pwm(13);","servo(8);","servo(9);",
		"encoder(3);","encoder(4);",
		"latency();",
		"sabertooth1(X2);"];
	*/

	_this.status_message(" Getting device configs from superstar...");

	superstar_get(_this.robot,"config",
		function(config){
			_this.last_config=config;

			if (config.counter!==undefined) {
				_this.status_message(" Got valid device configs from superstar: "+JSON.stringify(config));
				devices=devices.concat(config.configs);
			} else {
				_this.status_message(" Pilot needs to configure robot on superstar: "+_this.robot.name);
			}

			var d=0; // device counter
			var next_device=function() {
				d++;
				if (d<devices.length) {
					_this.arduino_setup_device(devices[d],next_device);
				} else { // configured last device
					_this.arduino_check_RAM();
				}
			}
			_this.arduino_setup_device(devices[0],next_device);
		});
}

// Configure this one device on the Arduino
connection_t.prototype.arduino_setup_device=function(device_name_args,callback)
{
	var _this=this;
	var name_regex=/^[^(]*/; // regex to match name of device (up to args)
	var device_name=device_name_args.match(name_regex)[0];
	_this.status_message(" Adding device name '"+device_name+"'"); // pure debug
	_this.device_names.push(device_name);
	_this.serial_send_ascii(device_name_args+"\n",function() {
		var read_to_end=function(line) {
			status=parseInt(line); // first int on line is status
			if (status==1) { // done with that device
				callback();
			} else {
				_this.status_message(" Arduino device setup: "+line);
				_this.serial_read_line(read_to_end);
			}
		}
		_this.serial_read_line(read_to_end);
	});
}

// Check Arduino free RAM after configuring devices
connection_t.prototype.arduino_check_RAM=function()
{
	var _this=this;
	_this.serial_send_ascii("ram?\n", function() {
		_this.serial_read_line( function(ram_line) {
			_this.status_message(" RAM free: "+ram_line);

			_this.arduino_setup_comms();
		} )
	} );
}

// Arduino is now configured--switch to binary comms
connection_t.prototype.arduino_setup_comms=function()
{
	var _this=this;

	_this.arduino_setup_device("serial_controller();", function() {
		_this.read_packet=_this.arduino_sensor_packet;
		_this.arduino_setup_complete();
	} );
}

/**
	Arduino binary blob <-> web JSON mapping via array of property strings.
Really these should be downloaded from the firmware, not hardcoded here.

Format for each property string:
	json path
	optional array size
	optional # meaning indexed by this device number
	<
		u for unsigned
		s for signed
		bit count
	>

E.g., analog#<u16> puts an unsigned 16-bit value into analog[0], then [1], ...

To traverse, see https://gist.github.com/wmbenedetto/5078940
*/

// Hash from device name to array of sensor property strings
connection_t.sensor_property_list={

"analog":["analog#<u16>"],
"blink":["blink#<u8>"],
"bms":["battery.charge<u8>","battery.state<u8>"],
"encoder":["encoder_raw#<u16>"],
"heartbeat":["heartbeats<u8>"],
"latency":["latency<u8>"],
"neopixel":[],
"pwm":[],
"serial_controller":[],
"servo":[],
"ultrasonic_sensor":["ultrasonic#<u8>"],
"wheel_encoder":["wheel.left<u16>","wheel.right<u16>", "wheel.wheelbase<u16>"],



// All these motor controllers have no sensor values
"sabertooth1":[],
"sabertooth2":[],
"bts":[],

// The create has a ton of sensors:
"create2":[
	"mode<u8>",
	"bumper<u8>",
	"battery.state<u8>",
	"battery.temperature<s8>",
	"battery.charge<s16>",
	"encoder.L<u16>",
	"encoder.R<u16>",
	"floor[4]<u8>",
	"light[6]<u8>",
	"light_field<u8>",
	"buttons<u8>",
],
};

// Hash from device name to array of command property strings
//   (same format as above)
connection_t.command_property_list={
"analog":[],
"bms":[],
"blink":["blink#<u8>"],
"encoder":[],
"heartbeat":[],
"latency":[],
"neopixel":[
	"neopixel#.color.r<u8>%=255",
	"neopixel#.color.g<u8>%=0",
	"neopixel#.color.b<u8>%=0",
	"neopixel#.accent.r<u8>%=0",
	"neopixel#.accent.g<u8>%=255",
	"neopixel#.accent.b<u8>%=255",
	"neopixel#.start<s8>=4",
	"neopixel#.repeat<u8>=8",
	"neopixel#.state<u8>=0"
],
"pwm":["pwm#<u8>%"],
"serial_controller":[],
"servo":["servo#<u8>=90"],
"ultrasonic_sensor":[],
"wheel_encoder":[],

// All the motor controllers have the same command interface:
"sabertooth1":["L<s16>%","R<s16>%"],
"sabertooth2":["L<s16>%","R<s16>%"],
"bts":["L<s16>%","R<s16>%"],
"create2":["L<s16>%","R<s16>%"],
};


// Persistently request this superstar path, sending
//   the resulting JSON object to this function.
connection_t.prototype.network_getnext=function(path,on_success)
{
	var _this=this;

	superstar.get(path,function(data)
	{
		if(on_success)
			on_success(data);
	});

	var func=function()
	{
		superstar.get_next(path,function(data)
		{
			if(_this.connected)
			{
				if(on_success)
					on_success(data);
				func();
			}
		});
	};

	func();
}


// Arduino is fully configured and communicating
connection_t.prototype.arduino_setup_complete=function()
{
	var _this=this;
	_this.status_message("  arduino setup complete... sending command packet");
	_this.arduino_send_packet();

	//Set up network comms:
	_this.network_getnext(robot_to_starpath(this.robot)+"pilot/power",function(power)
	{
		for (var field in power)
			_this.power[field]=power[field];
		console.log("power: "+JSON.stringify(power));
	});

	_this.network_getnext(robot_to_starpath(this.robot)+"config",function(config)
	{
		//need to reconfigure Arduino
		if(config.counter!=_this.last_config.counter)
			_this.reconnect();
	});
}

// Look up all the properties for our currently configured set of devices,
//   and call handle_property on each.
connection_t.prototype.walk_property_list=function(property_list,handle_property)
{
	var _this=this;
	var counts={};
	//console.log(JSON.stringify(_this.device_names));
	for (var devi in _this.device_names) {
		var dev=_this.device_names[devi];
		var props=property_list[dev];
		//console.log("|"+dev+"|"+props+"|");
		if (!props)
		{ // Can't find device in list--should be there though...
			for (var pli in property_list)
				_this.status_message("  Valid device '"+pli+"'");
			_this.bad("Device type '"+dev+"' not in property list! (Do you need to update this app to match your firmware?)");

		}

		// Update device counter, for stuff like servo# -> servo[0]
		var count=0;
		if (counts[dev]!==undefined) count=counts[dev]+1;
		counts[dev]=count;

		// Loop over device properties
		for (var propi in props) {
			// Loop over copies, for stuff like lights[4]
			var copies=1;
			var prop=props[propi];
			var copy_arr=prop.match(/\[([0-9]+)\]/);
			if (copy_arr && copy_arr[1]) copies=parseInt(copy_arr[1]);

			for (var copy=0;copy<copies;copy++) {
				// For array indexing here,
				//  we use .[4] instead of [4],
				//  to match how we parse JSON, with [field].
				var prop_copy=prop.replace(/\[[0-9]+\]/,".["+copy+"]");
				var prop_count=prop_copy.replace(/#/,".["+count+"]");
				handle_property(dev,prop_count);
			}
		}
	}
}

// Extract name string from Arduino property
//   e.g., foo.bar<u8> returns "foo.bar"
connection_t.prototype.arduino_property_name=function(property) {
	var type_regex=/<.*$/;
	var name_str=property.replace(type_regex,"");
	if (!name_str) _this.bad("Property '"+property+"' has invalid type "+name_str+" (firmware bug?)");
	return name_str;
}

// Extract property type string from Arduino property
//   e.g., foo.bar#<u8> returns "u8"
connection_t.prototype.arduino_property_type=function(property) {
	var type_regex=/<([us][0-9]*)>/;
	var type_str=property.match(type_regex)[1];
	if (!type_str) _this.bad("Property '"+property+"' has invalid type "+type_str+" (firmware bug?)");
	return type_str;
}

// Extract byte count from Arduino property
//   e.g., foo.bar#<u8> returns 1 (byte) for the 8-bit value
connection_t.prototype.arduino_property_bytecount=function(property) {
	var _this=this;
	var type_str=_this.arduino_property_type(property);
	var bitcount=parseInt(type_str.substring(1));
	if (isNaN(bitcount) || bitcount==0 || bitcount%8!=0) _this.bad("Property '"+property+"' has invalid size "+bitcount+" (firmware bug?)");
	return bitcount/8;
}

// Does this Arduino property come in as a percentage?
//   e.g., foo.bar<u8>% returns true
connection_t.prototype.arduino_property_percent=function(property) {
	var _this=this;
	var percent_regex=/[%]/;
	if (property.match(percent_regex)) return true;
	else return false;
}

// Extract default value count from Arduino property
//   e.g., foo.bar#<u8>=3 returns the 3 as a number
//   If no default is listed, returns 0.
connection_t.prototype.arduino_property_default=function(property) {
	var _this=this;
	var def_regex=/=([0-9]*)/;
	var def_match=property.match(def_regex);
	if (def_match) {
		var def_str=def_match[1];
		var v=parseInt(def_str);
		if (isNaN(v) ) _this.bad("Property '"+property+"' has invalid size "+bitcount+" (firmware bug?)");
		return v;
	} else { // no default value
		return 0;
	}
}


// Read values from byte array in different sizes
connection_t.read_bytes_as={
"u8":	function(buf,idx) {
		return buf[idx];
	},
"s8":	function(buf,idx) {
		var u8=buf[idx];
		return u8<<24>>24; // Shift up to 32-bit sign bit and back: see http://blog.vjeux.com/2013/javascript/conversion-from-uint8-to-int8-x-24.html
	},
"u16":	function(buf,idx) {
		return buf[idx]+(buf[idx+1]<<8);
	},
"s16":	function(buf,idx) {
		var u16=buf[idx]+(buf[idx+1]<<8);
		return u16<<16>>16; // Shift up to 32-bit sign bit and back
	},
};

// Write values to byte array from different sizes
connection_t.write_bytes_as={
"u8":	function(buf,idx,value) {
		if (value<0) value=0;
		if (value>255) value=255;
		buf[idx]=value&0xff;
	},
"s8":	function(buf,idx,value) {
		if (value<-128) value=-128;
		if (value>127) value=127;
		buf[idx]=value&0xff;
	},
"u16":	function(buf,idx,value) {
		if (value<0) value=0;
		if (value>65535) value=65535;
		buf[idx]=value&0xff;
		buf[idx+1]=(value>>8)&0xff;
	},
"s16":	function(buf,idx,value) {
		if (value<-32768) value=-32768;
		if (value>32767) value=32767;
		buf[idx]=value&0xff;
		buf[idx+1]=(value>>8)&0xff;
	},
};

// Convert string array index to int array index
//   (or if not an array index, return unmodified)
connection_t.prototype.JSON_array_index=function(str_idx)
{
	var _this=this;
	if (str_idx[0]=='[') {
		var idx=str_idx.match(/\[([0-9]+)\]/)[1];
		var idx_int=parseInt(idx);
		return idx_int;
	} else {
		return str_idx;
	}
}

// Write obj.property=value;
connection_t.prototype.write_JSON_property=function(obj,property,value)
{
	var _this=this;
	var name=_this.arduino_property_name(property);
	var fieldlist=name.split(".");
	for (var i=0;i<fieldlist.length-1;i++) {
		var f=fieldlist[i];
		var is_array=false;
		if (fieldlist[i+1][0]=='[') { // next index is an array
			is_array=true;
		}
		f=_this.JSON_array_index(f);

		if (obj[f]!==undefined) obj=obj[f];
		else {
			if (is_array) obj=obj[f]=[];
			else	obj=obj[f]={};
		}
	}
	obj[_this.JSON_array_index(fieldlist[fieldlist.length-1])]=value;
}

// Return obj.property, or the default value if none exists
connection_t.prototype.read_JSON_property=function(obj,property)
{
	var _this=this;
	var name=_this.arduino_property_name(property); // e.g., foo.bar.[3]
	var fieldlist=name.split(".");
	for (var i=0;i<fieldlist.length-1;i++) {
		var f=fieldlist[i];
		var is_array=false;
		if (fieldlist[i+1][0]=='[') { // next index is an array
			is_array=true;
		}
		f=_this.JSON_array_index(f);

		if (obj[f]!==undefined) obj=obj[f];
		else {
			if (is_array) obj=obj[f]=[];
			else	obj=obj[f]={};
		}
	}
	var f=fieldlist[fieldlist.length-1];
	f=_this.JSON_array_index(f);
	if (obj[f]!==undefined) return obj[f];
	// else return default value
	return _this.arduino_property_default(property);
}

// Build a pilot packet and send it to the Arduino
connection_t.prototype.arduino_send_packet=function()
{
	var _this=this;

	if (_this.sends_in_progress)
	{ // haven't finished sending last packet yet
		_this.status_message("  skipping send_packet due to outstanding data");
		return;
	}

	// How big is the command packet?
	var cmd_bytes=0;
	_this.walk_property_list(connection_t.command_property_list,function(device,property) {
		// _this.status_message(" device "+device+" has property "+property);
		cmd_bytes+=_this.arduino_property_bytecount(property);
	} );

	// Fill a packet that size
	var cmd_data=new Uint8Array(cmd_bytes);
	var idx=0; // current output index
	_this.walk_property_list(connection_t.command_property_list,
	  function(device,property) {
		var value=_this.read_JSON_property(_this.power,property);
		_this.write_JSON_property(_this.sensors.power,property,value);
		//console.log("|"+device+"|"+value+"|");

		//MOTOR SCALING
		if(_this.arduino_property_percent(property))
			value*=255/100; // comes in as a percent, leaves as a byte value

		var motor_limit_value=255;
		if(value>motor_limit_value)
			value=motor_limit_value;
		if(value<-motor_limit_value)
			value=-motor_limit_value;

		var datatype=_this.arduino_property_type(property);
		connection_t.write_bytes_as[datatype](cmd_data,idx,value);
		_this.status_message("  Wrote "+property+" as "+cmd_data[idx]);
		idx+=_this.arduino_property_bytecount(property);
	  }
	);

	// Send to Arduino
	_this.serial_send_packet(0xC,cmd_data,function() {
		// maybe check if we delayed a send previously here?
	} );
}




// Parse this binary sensor data packet from the Arduino,
//   convert to JSON, and send to superstar.
connection_t.prototype.arduino_recv_packet=function(p)
{
	var _this=this;

	_this.status_message("Arduino incoming sensor data "+p);
	var idx=0;
	_this.walk_property_list(connection_t.sensor_property_list,
	  function(device,property) {
		var datatype=_this.arduino_property_type(property);
		var value=connection_t.read_bytes_as[datatype](p.data,idx);
		_this.write_JSON_property(_this.sensors,property,value);


		idx+=_this.arduino_property_bytecount(property);
	  }
	);

	if(_this.sensors.encoder) // Assuming create 2
	{

		if(_this.old)
		{
			function wraparound_fix(old_value, new_value)
			{
				return (0xff&(new_value-old_value+128))-128;
			}

			var m_per_tick=0.000444;
			var wheelbase=.235;
			_this.move_wheels(wraparound_fix(_this.old.L, _this.sensors.encoder.L)*m_per_tick,
								wraparound_fix(_this.old.R, _this.sensors.encoder.R)*m_per_tick,
								wheelbase);
		}
		_this.old={};
		_this.old.L=_this.sensors.encoder.L;
		_this.old.R=_this.sensors.encoder.R;

		_this.sensors.location=_this.location;
		_this.sensors.location.angle=_this.angle;
	}

	if (idx!=p.length) _this.bad("Arduino sensor packet length mismatch: got "+p.length+", expected "+idx+" (firmware/app mismatch?)");

	_this.status_message("	sensors="+JSON.stringify(_this.sensors,null,'	'));


	// Upload new sensor data to network:
	superstar_set(_this.robot,"sensors",_this.sensors);

/*
	// Send sensor data to superstar, and grab pilot commands with set & mget:
	var robotName=_this.robot.school+"/"+_this.robot.name;
	superstar_generic(_this.robot,
		"sensors?set="+encodeURIComponent(JSON.stringify(_this.sensors))
		+"&get="+robotName+"/pilot,"+robotName+"/config"
	,
	function(pilot_and_config_str) {
		var pilot_and_config=JSON.parse(pilot_and_config_str);
		if (pilot_and_config.length!=2) _this.bad("Invalid pilot & config data back from superstar");
		var pilot=pilot_and_config[0];
		_this.status_message("	pilot="+JSON.stringify(pilot,null,'	'));
		var config=pilot_and_config[1];
		if (config.counter!=_this.last_config.counter)
		{ // need to reconfigure Arduino
			_this.reconnect();
		}

		for (var field in pilot.power) {
			_this.power[field]=pilot.power[field];
		}
	} );
*/
}


// Packet of sensor(?) data just arrived from the Arduino
connection_t.prototype.arduino_sensor_packet=function(p)
{
	var _this=this;
	switch (p.command) {
	case 0xC: // sensor data
		_this.arduino_recv_packet(p);
		setTimeout(function() {
			_this.arduino_send_packet(); // send command data in response
		}, _this.serial_delay_ms); // limit CPU usage by waiting to send response
		break;
	case 0xE: // Error code
		_this.bad("Arduino firmware error packet "+p);
		break;
	case 0xD: // Debug data
		_this.status_message("Arduino firmware debug packet "+p);
		break;
	case 0x0: // Ping
		_this.status_message("Arduino ping packet "+p);
		_this.arduino_send_packet(); // send command data in response
		break;
	default: // ??
		_this.status_message("Arduino unexpected packet "+p);
		break;
	};
}




/***************** Low-level serial data I/O ********************/
// Callback from serial API: error on serial port
connection_t.prototype.serial_callback_onReceiveError=function(info)
{
	var _this=this;
	_this.status_message("Serial error: "+info.error);
	// now what?!
	_this.gui_disconnect(_this.port_name);
}


// Callback from serial API: incoming serial data
connection_t.prototype.serial_callback_onReceive=function(info)
{
	var _this=this;
	_this.status_message("Serial data received: "+info.data.byteLength+" bytes");
	if (!_this.connected()) return;

	// parse?
	var buffer=info.data; // ArrayBuffer
	var arr=new Uint8Array(buffer);
	for (var i=0;i<arr.length;i++) {
		var v=arr[i];
		var c=String.fromCharCode(v);

		if (_this.show_debug_bytes) { // show incoming bytes
			_this.status_message("	received  \t"+v+"  \t"+c);
		}

		if (_this.read_line_callback)
		{ // line based
			if (v==13) {} // cursed windows newline--ignore it
			else if (v==10) // real newline
			{
				// SUBTLE: callback will register new callbacks,
				//  so save state before calling.
				var callback=_this.read_line_callback;
				var line=_this.read_line;

				_this.read_line_callback=null;
				_this.read_line=""; // reset line

				// Per-line debugging:
				_this.status_message(" received serial: "+line);

				callback(line); // call user's callback
			}
			else { // normal data part of the line
				_this.read_line+=c;
			}
		}
		else if (_this.read_packet)
		{ // packet based
			_this.serial_read_packet(v);
		}
		else if (_this.serial_startup) {
			// crap in serial buffer beforehand
		}
		else { // who ordered this?
			_this.bad("Unexpected serial data from Arduino: "+v+"  \t"+c);
		}
	}
}

// Read a line of ASCII text from the serial port,
//   and pass it to this callback (less newline)
connection_t.prototype.serial_read_line=function(line_callback)
{
	var _this=this;
	if (_this.read_line_callback) _this.bad("Cannot wait for two lines at once!");
	_this.read_line_callback=line_callback;
	// serial onReceive will call this once the line arrives
}

// Send a brick of binary data out the serial port.
//  Serial port is blocked until we call the done_callback.
connection_t.prototype.serial_send=function(array_like,done_callback)
{
	var _this=this;
	if (!_this.connected()) return;
	if (_this.sends_in_progress!=0) _this.bad("Cannot overlap send calls!");
	_this.sends_in_progress++;

	var out=new Uint8Array(array_like);

	if (_this.show_debug_bytes) { // show sent bytes
		_this.status_message("Serial data sending: "+out.byteLength+" bytes");
		for (var i=0;i<out.byteLength;i++)
			_this.status_message("	sending  \t"+out[i]+"  \t"+String.fromCharCode(out[i]));
	}

	_this.serial_api.send(_this.connection,out.buffer,
		function(info) {
			if (info.error) {
				_this.bad("Serial port send error: "+info.error);
			}
			else if (info.bytesSent!=out.byteLength) {
				_this.bad("Serial port sent only "+info.bytesSent+" bytes, should be "+out.byteLength+" bytes!");
			}
			else {
				_this.sends_in_progress--;
				done_callback();
			}
		}
	);
}

// Send a string of ASCII text, and
//   call this callback after it's been sent.
connection_t.prototype.serial_send_ascii=function(str,done_callback)
{
	var _this=this;
	// from http://stackoverflow.com/questions/6965107/converting-between-strings-and-arraybuffers
	var buf=new ArrayBuffer(str.length);
	var bufView=new Uint8Array(buf);
	for (var i=0, strLen=str.length; i<strLen; i++) {
		var c=str.charCodeAt(i);
		if (c<=255) bufView[i]=c;
		else _this.bad("serial_send_ascii with non-ASCII character code "+str[i]+"=="+c);
	}
	_this.serial_send(buf,done_callback);
}

/***** Binary A-packet (see serial_packet.h) serial data format ******/

// Parse this incoming serial data byte (int) as an A-packet.
//   Call the read_packet callback once packet is complete.
//   Copied directly from C++serial_packet.h read_packet function.
connection_t.prototype.serial_read_packet=function(c)
{
	var _this=this;
	var p=_this.read_packet_data;
	p.valid=false;

	// pseudo-enum
	var STATE_START=0;
	var STATE_LENGTH=1;
	var STATE_PAYLOAD=2;
	var STATE_END=3;

	switch (_this.read_state) {
	case 0: /* start byte */
		if ((c&0xf0)==0xa0) { // valid start code
			_this.read_index=0;
			_this.read_sumpay=0;
			p.length=c&0x0f;
			if (p.length>=_this.max_short_length)
			{
				_this.read_state=STATE_LENGTH; // need real length byte
			}
			else if (p.length>0) {
				_this.read_state=STATE_PAYLOAD; // short payload data
				p.data=new Uint8Array(p.length);
			} else { // length==0, no payload
				_this.read_state=STATE_END;
			}
		}
		else _this.status_message("Serial A-packet unexpected byte: \t"+c+"  \t"+String.fromCharCode(c));

		break;
	case STATE_LENGTH: /* (optional) length byte */
		p.length=c;
		_this.read_state=STATE_PAYLOAD;
		p.data=new Uint8Array(p.length);
		if(p.length==0) {
			_this.read_state=STATE_END;
		}
		break;
	case STATE_PAYLOAD: /* payload data */
		_this.read_sumpay+=c;
		p.data[_this.read_index]=c;
		_this.read_index++;
		if (_this.read_index>=p.length) { // that was last byte of payload!
			_this.read_state=STATE_END;
		}
		break;
	case STATE_END: /* end byte */
		{
		_this.read_state=STATE_START;
		p.command=c>>4;
		var checksum=0xf&(p.length+p.command+_this.read_sumpay+(_this.read_sumpay>>4));
		var checkread=0xf&(c);
		if (checkread==checksum) { /* checksum match--valid packet! */
			p.valid=true;
			_this.read_packet(p); // report packet to callback
		}
		else _this.status_message("Serial A-packet checksum error: "+checkread+" vs "+checksum);

		}
		break;
	default: /* only way to get here is memory corruption!  Reset. */
		_this.read_state=0; break;
	};
}

// Send a binary A-packet with this Uint8Array payload, and
//   call this callback after it's been sent.
connection_t.prototype.serial_send_packet=function(command,array_data,done_callback)
{
	var _this=this;
	var length=array_data.byteLength;
	var out_len=0;
	if (length<_this.max_short_length) out_len=1+length+1;
	else out_len=2+length+1; // extra length byte at start
	var out=0; // index in output array
	var write_data=new Uint8Array(out_len);

	// Send start of packet
	var start=0xA0;  // sync code, length in low bits
	if (length<_this.max_short_length)
	{ // send short packets in one big buffer
		write_data[out++]=start+length;
	}
	else { // long packet, with length byte
		write_data[out++]=start;
		write_data[out++]=length;
		if (length>255) _this.bad("serial_send_packet data too big to send!");
	}

	// Copy payload data & compute checksum
	var sumpay=0;
	for (var i=0;i<length;i++) {
		sumpay+=array_data[i];
		write_data[out++]=array_data[i];
	}
	var checksum=0xf&(length+command+sumpay+(sumpay>>4));

	// Send end of packet
	var end=(command<<4)+(checksum);
	write_data[out++]=end;

	if (out!=out_len) _this.bad("serial_send_packet packet length logic error!");

	_this.serial_send(write_data,done_callback);
}

connection_t.prototype.save=function()
{
	var _this=this;
	var robot=this.robot;
	robot.serial=this.port_name;
	chrome.storage.local.set({"robot":robot},
		function(){_this.status_message("Saved robot.");});
}

connection_t.prototype.load=function()
{
	var _this=this;

	chrome.storage.local.get("robot",
		function(data)
		{
			_this.status_message("Loaded robot:  "+JSON.stringify(data));

			if(!data.superstar)
				data.superstar="robotmoose.com";

			if(_this.on_name_set&&valid_robot(data.robot))
				_this.on_name_set(data.robot);
			else
				_this.on_name_set();
		});
}


connection_t.prototype.move_wheels=function(left, right, wheelbase)
{
	var _this=this;
	// Extract position and orientation from absolute location
	var P=_this.location; // position of robot (center of wheels)
	var ang_rads=_this.angle*Math.PI/180; // 2D rotation of robot

	// Reconstruct coordinate system and wheel locations
	var FW=new vec3(Math.cos(ang_rads), Math.sin(ang_rads), 0.0); // forward vector
	var UP=new vec3(0,0,1); // up vector
	var LR=FW.cross(UP); // left-to-right vector
	var wheel=new Array(2);
	wheel[0]=P.sub(LR.mul(0.5*wheelbase));
	wheel[1]=P.add(LR.mul(0.5*wheelbase));

	// Move wheels forward by specified amounts
	wheel[0]=wheel[0].add(FW.mul(left));
	wheel[1]=wheel[1].add(FW.mul(right));

	// Extract new robot position and orientation
	P=(wheel[0].add(wheel[1])).mul(0.5);
	LR=(wheel[1].sub(wheel[0])).normalize();
	FW=UP.cross(LR);
	ang_rads=Math.atan2(FW.y, FW.x);

	// Put back into absolute location
	_this.location=P;
	_this.angle=180/Math.PI*ang_rads;
}

//robot_localization.prototype.
