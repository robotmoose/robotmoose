// frontend robot simulator


function robot_sim_t()
{
	
	_this = this;
	this.sim = true;
	this.superstar=null; // <- means "same server as this page"
	this.year="2016";
	this.school="uaf";
	this.name="simulation";
	this.auth="";

	this.pilot={};
	this.pilot_status={};
	
	this.active_experiment="";
	this.experiments = {};
	this.experiments_list = [];
	
	this.gui=[];
	
	this.config=[];
	
	if (localStorage)
	{
		if (localStorage.active_experiment) this.active_experiment = localStorage.getItem("active_experiment");
	
		if (localStorage.experiments) this.experiments= JSON.parse(localStorage.getItem("experiments"));
	
		if(localStorage.experiments_list) this.experiments_list = JSON.parse(localStorage.getItem("experiments_list"));
		
		if(localStorage.sim_gui) this.gui = JSON.parse(localStorage.getItem("sim_gui"));
		
		if(localStorage.sim_config) 
		{
			this.config = JSON.parse(localStorage.getItem("sim_config"));
		}
	}

			
	this.options=[];
	this.options.push("analog P");
	this.options.push("servo P");
	this.options.push("pwm P");
	this.options.push("create2 S");
	this.options.push("neato SP");
	this.options.push("neopixel PC");
	this.options.push("bts PPPP");
	this.options.push("bms");
	
	
	this.pilot={};
	this.sensors={};
	this.devices = ["heartbeat();"];
	this.device_names=[];
	
	this.heartbeats = 0;
	var location = {};
	location.ID = 0;
	location.angle = 0;
	location.count = 0;
	location.x = 0.0;
	location.y = 0.0;
	location.z = 0.0;
	
	this.sensors.heartbeats = this.heartbeats;
	this.sensors.location = location;

	this.wheelbase = 0.3;
	this.wheel=[];
	this.reset_wheels();
}
	
/*
// Move robot to location
robot_sim_t.prototype.change_location=function(x_cord, y_cord, z_cord)
{
	if (!x_cord) x_cord = 0;
	if (!y_cord) y_cord = 0;
	if (!z_cord) z_cord = 0;
	
	this.wheel[0]=new vec3(x_cord,y_cord+0.5*this.wheelbase,z_cord);
	this.wheel[1]=new vec3(x_cord,y_cord-0.5*this.wheelbase,z_cord);	
	
	this.drive_wheels(0.0,0.0);
}
*/

// Reset positions of wheels:
// Taken from robot_2wd
robot_sim_t.prototype.reset_wheels=function()
{
	this.wheel[0]=new vec3(0.0,+0.5*this.wheelbase,0.0);
	this.wheel[1]=new vec3(0.0,-0.5*this.wheelbase,0.0);
	
	// Robot coordinate system:
	this.P=new vec3(0,0,0); // position of robot (between wheels)
	this.UP=new vec3(0,0,1); // Z is up
	this.LR=new vec3(0,-1,0); // left-to-right wheel
	this.FW=new vec3(1,0,0); // drive forward
	
	this.drive_wheels(0.0,0.0); // set up coordinates
}



// Drive the robot's wheels this far.
// Taken from robot_2wd
robot_sim_t.prototype.drive_wheels=function(L,R)
{	
	// Drive wheels forward
	this.wheel[0].pe(this.FW.t(L));
	this.wheel[1].pe(this.FW.t(R));

	// Enforce wheelbase
	this.P=this.wheel[0].p(this.wheel[1]).te(0.5);
	this.LR=this.wheel[1].m(this.wheel[0]); // left-to-right
	this.LR.normalize();
	this.FW=new vec3(0,0,1).cross(this.LR).normalize();  // forward
	var wheeloff=this.LR.clone();
	wheeloff.te(this.wheelbase*0.5);
	this.wheel[0]=this.P.m(wheeloff);
	this.wheel[1]=this.P.p(wheeloff);
	
	// Robot's Z rotation rotation, in radians
	this.angle_rad=Math.atan2(this.FW.y,this.FW.x);
	this.angle=180.0/Math.PI*this.angle_rad;

	if (this.sensors.location);
	{
	this.sensors.location.angle=this.angle;
	this.sensors.location.x = this.P.x;
	this.sensors.location.y = this.P.y;
	this.sensors.location.z = this.P.z;
	}
}

	
	
robot_sim_t.prototype.update_pilot=function(pilot){
	this.pilot = pilot;
	this.sensors.power=this.pilot.power;
	if (!isNaN(this.pilot.power.L)&&!isNaN(this.pilot.power.R))
	{
		if (!this.loop) 
		{
			this.loop = true;
			this.drive_loop();
		}

	}
}

robot_sim_t.prototype.drive_loop=function(){

	var _this = this;
	var distance_per_power=0.0002;	

			if (this.pilot.power.L > 100.0) this.pilot.power.L = 100.0;
	if (this.pilot.power.L < -100.0) this.pilot.power.L = -100.0;
	if (this.pilot.power.R > 100.0) this.pilot.power.R = 100.0;
	if (this.pilot.power.R < -100.0) this.pilot.power.R = -100.0;
	
	var dist_L = this.pilot.power.L*distance_per_power;
	var dist_R = this.pilot.power.R*distance_per_power;
	
	this.drive_wheels(dist_L, dist_R);	

		
	if ((this.pilot.power.L==0&&this.pilot.power.R==0)||!robot_network.sim) this.loop = false;
	else 
	{
		setTimeout(function(){_this.drive_loop()}, 30);
	}
}


robot_sim_t.prototype.setup_sensors=function()
{
	var _this = this;
	this.clear_sensors();
	this.devices = ["heartbeat();"];
	this.device_names=[];
	this.devices = _this.devices.concat(_this.config.configs);
	
	var d=0; // device counter
	var next_device=function() {
		d++;
		if (d<_this.devices.length)
			_this.add_device(_this.devices[d],next_device);
	}
	_this.add_device(_this.devices[0],next_device);
}


robot_sim_t.prototype.add_device=function(device_name_args,callback)
{
	
	var _this=this;
	var name_regex=/^[^(]*/; // regex to match name of device (up to args)
	var device_name=device_name_args.match(name_regex)[0];
	_this.device_names.push(device_name);
	_this.add_sensor(device_name);
	callback();
}

robot_sim_t.prototype.add_sensor=function(device_name)
{
	var _this = this;
	switch (device_name) {
		case "analog":
			if(!this.sensors.analog) this.sensors.analog=[];
			this.sensors.analog.push(0);
			break;
			
		case "servo" :
			if(!this.sensors.power) this.sensors.power={};
			if(!this.sensors.power.servo) this.sensors.power.servo =[];
			this.sensors.power.servo.push(70);
			break;
		case "pwm" :
			if(!this.sensors.power) this.sensors.power={};
			if(!this.sensors.power.pwm) this.sensors.power.pwm =[];
			this.sensors.power.pwm.push(10);
			break;
			
		case "create2":
			this.sensors.battery ={};
			this.sensors.battery.charge = 0;
			this.sensors.battery.state = 0;
			this.sensors.battery.temperature = 0;
			this.sensors.bumper = 0;
			this.sensors.buttons = 0;
			this.sensors.encoder = {};
			this.sensors.encoder.L = 0;
			this.sensors.encoder.R = 0;
			this.sensors.floor=[0,0,0,0];
			this.sensors.light=[0,0,0,0,0,0];
			this.sensors.light_field=0;
			this.sensors.mode=0;
			if(!this.sensors.power) this.sensors.power={};
			this.sensors.power.L = 0.0;
			this.sensors.power.R=0.0;
			break;
			
		case "neato":
			if(!this.sensors.lidar) this.sensors.lidar={};
			this.sensors.lidar.change = 0;
			this.sensors.lidar.depth = new Array(360).fill(0);
			this.sensors.lidar.errors=0;
			this.sensors.lidar.rpm=0;
			this.sensors.lidar.scale=0.0001;
			break;
			
		case "neopixel":
			if(!this.sensors.power)this.sensors.power={}
			if(!this.sensors.power.neopixel) this.sensors.power.neopixel=[];
			var neopixel = new Object();
			neopixel.accent={b:0,g:0,r:0};
			neopixel.color={b:0,g:0,r:0};
			neopixel.repeat=0;
			neopixel.start=0;
			neopixel.state=0;
			this.sensors.power.neopixel.push(neopixel);
			break;
		case "bts":
			break;
		case "bms":
			break;
			
			
	}
	
	
}

robot_sim_t.prototype.clear_sensors=function()
{
	var heartbeats = this.sensors.heartbeats;
	var location = this.sensors.location;
	this.sensors={};
	this.sensors.heartbeats=heartbeats;
	this.sensors.location=location;
}

function sim_set_experiment(robot, active_experiment, data, on_success)
{
	if ((robot.experiments_list.indexOf(active_experiment)==-1)&&active_experiment) 
	{
		robot.experiments_list.push(active_experiment);
	}
	robot.active_experiment = active_experiment;
	robot.experiments[active_experiment] = data;

	if(localStorage)
	{
		localStorage.setItem("active_experiment", robot.active_experiment);
		localStorage.setItem("experiments", JSON.stringify(robot.experiments));
		localStorage.setItem("experiments_list", JSON.stringify(robot.experiments_list));
	}
	if(on_success) on_success();
}

function sim_get_experiment(robot, experiment, on_success)
{
	on_success(robot.experiments[experiment])
}

function sim_get(robot,path,on_success, on_error)
{
	if (path === "options")
	{

		on_success(robot.options);
	}
	else if (path === "config")
	{

		on_success(robot.config);
	}
	else if (path === "gui")
	{
		on_success(robot.gui);
	}
	else if (path === "active_experiment")
	{
		on_success(robot.active_experiment);
	}
	else console.log("attempted sim_get: " + path);

}

function sim_set(robot, path, json, on_success)
{
	
	if (path === "config")
	{
		robot.config = json;
		if(localStorage) localStorage.setItem("sim_config", JSON.stringify(json));
		robot.setup_sensors();

	}
	else if (path === "frontendStatus")
	{
		robot.pilot_status = json[0];
		robot.heartbeats = robot.pilot_status["heartbeats"];
		robot.sensors.heartbeats = robot.heartbeats;

	}
	else if (path === "active_experiment")
	{
		robot.active_experiment = json;
		if (!robot.experiments[json]) robot.experiments[json]="";
	}
	else if (path === "gui")
	{	
		robot.gui = json;
		if(localStorage) localStorage.setItem("sim_gui",JSON.stringify(json));
	}
	else console.log("attempted sim_set: " + path);

}
