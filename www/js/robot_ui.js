/**
  This is the new 2015-07 modular robot interface, with a row of buttons on top,
  and moveable "doorways" showing each robot interface element.
*/

//to add a widget, add an entry to create_widgets.
//if the widget has a download method, it will be called.
//if the widget has a upload method, add the callback in create_widgets.

function robot_ui_t(gui_div,menu_div,modal_div)
{
	if(!gui_div||!menu_div||!modal_div)
		return null;
	this.gui_div=gui_div;
	this.menu_div=menu_div;
	this.modal_div=modal_div;

	this.robot=
	{
		year:null,
		school:null,
		name:null,
		auth:null
	};
	robot_network.sim=false;

	this.disconnected_text="<font style='color:red;'>Not connected.</font>";

	this.state_runner=new state_runner_t();
	this.menu=null;
	this.connect_menu=null;
	this.gui=
	{
		interval:null,
		old:""
	};
	this.doorways={};
	this.widgets={};

	this.create_gui();

	this.request_uuid();

	var _this=this;
	var options=parse_uri(location.search);
	validate_robot_name(options.robot,
		// good robot:
		function(robot)
		{
			_this.connect_menu.onconnect(robot);
		},
		// bad robot:
		function(robot)
		{
			if(robot)
			{
				localStorage.previous_year=robot.year;
				localStorage.previous_school=robot.school;
				localStorage.previous_robot=robot.name;
				_this.connect_menu.show_auth_error();
			}

			_this.connect_menu.show();
		});
	this.pilot_heartbeat = new pilot_status_t(this);
}

robot_ui_t.prototype.create_menus=function()
{
	var _this=this;

	this.menu=new robot_menu_t(this.menu_div);
	this.connect_menu=new modal_connect_t(this.modal_div);

	this.menu.get_status_area().innerHTML=this.disconnected_text;
	this.menu.create
	(
		"Connect",
		function(){_this.connect_menu.show();},
		"Connect to a new robot over the network",
		'transfer'
	);

	this.connect_menu.onconnect=function(robot)
	{
		if(robot)
		{
			_this.robot=JSON.parse(JSON.stringify(robot));

			if(robot.sim)
				_this.robot=robot;
			else
				robot_network.sim="";


			clearInterval(_this.gui.interval);
			_this.gui.interval=null;
			_this.menu.get_status_area().innerHTML="Connected<br/>"+
				_this.robot.year+"<br/>"+
				_this.robot.school+"<br/>"+
				_this.robot.name+"<br/>";
			_this.download_gui();
			//_this.widgets.chat.set_robot(robot);
			robot_network.set_robot(robot);
		}
	};
}


robot_ui_t.prototype.create_gui=function()
{
	this.create_menus();
	this.doorway_manager=new doorway_manager_t(this.gui_div,this.menu);
}

robot_ui_t.prototype.download_gui=function()
{
	if(!valid_robot(this.robot))
		return;

	var _this=this;


	var help_text_config =
	"<h3>When configuring devices:</h3>"
	+"<h4><b>wheel_encoder</b></h4>"
	+"<ul><li>Pin: Left wheel encoder pin</li><li>Pin: Right wheel encoder pin</li><li>"
	+"Number: Robot wheelbase <ul><li>Distance between wheel centers (mm)</li></ul></li></ul>"

	var help_text_pilot =
	"<h4>Sliders</h4>"
	+ "A slider will appear here for each pwm and servo device you configure in the Configure tab.<br><br>"
	+ "<h4>Steering Wheel</h4>"
	+ "A steering wheel will appear if you connect to a robot and configure it with a create2 device in the Configure tab.<br><br>"
	+ "To drive using the mouse, click and hold the wheel. The speed of the robot is determined by how far the cursor is from the center of the wheel.<br><br>"
	+ "The maximum speed is determined by the drive power slider<br><br>"
	+ "To drive using the keyboard, hold the mouse cursor over the wheel and drive using the keys w, a, s, d.<br><br>"

	var help_text_sensors =
	"<h3>How to Interpret Sensor Data</h3>"
	+ "<h4>Create 2 Sensors:</h4>"
	+ "<ul>"
	+ "<li>Mode:</li>"
	+ "<li>Bumper: Bumper data is represented by a 4-bit field. The two least significant "
	+ "bits are the front bumper data, and the two most significant bits are the wheel "
	+ "drop sensors.</li>"
	+ "<li>Floor: If any of the floor sensors are 0, then at least part of the robot is "
	+ "off of the floor.</li>"
	+ "<li>Light: The numbers represent the strength of the light sensors. The light sensors  "
	+ "are numbered 0 to 5, and are located at:"
	+ "<ul><li>0: Bumper Left</li><li>1: Bumper Front Left</li><li>2: Bumper Center Left</li>"
	+ "<li>3: Bumper Center Right</li><li>4: Bumper Front Right</li><li>5: Bumper Right</li></ul></li>"
	+ "</ul>";

	var help_text_states =
	//"<h3> Code Examples </h3>"
	"<h4>Basic</h4>"
	+ "print(\"This will appear to the right of the state\") <br>"
	+ "stop() <br>"
	+ "state = \"turnLeft\" // <i>Runs your state named \"turnLeft\"</i><br>"
	+ "<h4>UI</h4>"
	+ "label(\"This will appear in the UI\")<br>"
	+ "button(\"Press me\", \"turnLeft\")<br>"
	+ "power.L=slider(\"left wheel\", -100.0, 0.0, 100.0) <br>"
	+ "checkbox(\"Would you like to check this box?\")<br>"
	//+ "if (checkbox(\"Would you like to check this box?\")) { print(\"Yes!\") } <br>"
	+ "<h4>Drive</h4>"
	+ "forward(10)  // <i>Drive forward 10 centimeters</i><br>"
	+ "backward(5)  // <i>Drive backward 5 centimeters</i><br>"
	+ "left(45)  // <i>Turn left 45 degrees</i><br>"
	+ "right(90)  // <i>Turn right 90 degrees</i><br>"
	+ "drive(-50, 50)  // <i>Sets the speed of the left and right wheels</i><br>"
	+ "driveToPoint(2, 4)  // <i> Drive to point (2,4): 2 meters to the right and 4 meters up </i>"
	+ "<h4>Servos</h4>"
	+ "power.servo[0] = 90  // <i>Moves the first servo to position 90 degrees (range is 0 to 180)<br>"
	+ "<br><a href=\"/code/api/\" title=\"API\" target=\"new\"> <h4>Advanced</h4> </a>";
	//+ "<h4> Sensors </h4>"
	//+ "sensors.location.x - <i> Robot location in x-coordinate</i><br>"
	//+ "sensors.location.angle - <i> Direction robot is facing </i><br>"
	//+ "sensors.floor[2] - <i> Floor sensor 2 </i><br>";


	var help_text_map =
	"<h3>Driving on the Map</h3>"
	+ "Connect to a robot or simulation and configure a create2 device in the Configure tab.<br><br>"
	+ "You can drive by using the wheel in the Drive tab or by using code in the Code tab.<br><br>"
	+ "If the 3D robot is not responding, check that the robot is connected by opening the Sensors tab. If the heartbeats value is changing, the robot is connected.<br><br>"
	+ "<h3>Uploading a Map Image</h3>"
	+ "Click on the Upload Map Image button to use an image from your computer as the map.<br><br>"
	+ "You will be asked to choose a file from your computer and to enter the width and height of the space (in meters).<br><br>"
	+ "You can upload any number of map images and switch between them.<br><br>"
	+ "Uploaded images are not saved on the web server, so you will need to upload them again each time you connect to a robot.";


	var help_text_ui =
	"Run your Code to add elements to the UI <br>"
	+ "<h4>UI elements:</h4>"
	+ "label(\"This will appear in the UI\")<br>"
	+ "button(\"Press me\", state1)<br>"
	+ "slider(\"Slider!\", -1.0, 0.0, 1.0) <br>"
	+ "checkbox(\"Would you like to check this box?\")<br>"
	+ "<br><a href=\"/code/api/\" title=\"API\" target=\"new\"> <h4>Advanced</h4> </a>";

	var clear_out=function(div)
	{
		while(div.firstChild)
			div.removeChild(div.firstChild);
	}

	if(!this.robot.sim)
		superstar_get(this.robot,"gui",function(json)
		{
			_this.doorways=
			{
				config:_this.create_doorway("Configure","Set up robot hardware",help_text_config),
				pilot:_this.create_doorway("Drive","Manually drive the robot",help_text_pilot),
				sensors:_this.create_doorway("Sensors","Examine sensor data from robot",help_text_sensors),
				charts:_this.create_doorway("Charts", "Chart sensor data received from robot",null),
				states:_this.create_doorway("Code","Automatically drive the robot",help_text_states),
				map:_this.create_doorway("Map","See where the robot thinks it is",help_text_map),
				navigation: _this.create_doorway("Navigation", "Plan a path for the robot", null),
				video:_this.create_doorway("Video","Show the robot's video camera",null),
				UI:_this.create_doorway("UI","Customized robot user interface",help_text_ui),
				sound:_this.create_doorway("Sound","Play sounds on the backend to get attention",null),
				chat:_this.create_doorway("Chat","Chat with the caretaker of the robot.",null)
			};

			clear_out(_this.doorways.config.content);
			clear_out(_this.doorways.pilot.content);
			clear_out(_this.doorways.sensors.content);
			clear_out(_this.doorways.charts.content);
			clear_out(_this.doorways.states.content);
			clear_out(_this.doorways.map.content);
			clear_out(_this.doorways.navigation.content);
			clear_out(_this.doorways.video.content);
			clear_out(_this.doorways.UI.content);
			clear_out(_this.doorways.sound.content);
			clear_out(_this.doorways.chat.content);

			_this.doorway_manager.hide_all();
			_this.doorways.config.set_minimized(false);

			_this.doorway_manager.load(json);
			_this.create_widgets();

			for(let key in _this.widgets)
				if(_this.widgets[key].download)
					_this.widgets[key].download(_this.robot);

			_this.gui.interval=setInterval(function(){_this.run_interval();},100);
		});
	else
		sim_get(this.robot,"gui",function(json)
		{
			_this.doorways=
			{
				config:_this.create_doorway("Configure","Set up robot hardware",help_text_config, 'wrench'),
				pilot:_this.create_doorway("Drive","Manually drive the robot",help_text_pilot, 'dashboard'),
				sensors:_this.create_doorway("Sensors","Examine sensor data from robot",help_text_sensors, 'scale'),
				charts:_this.create_doorway("Charts", "Chart sensor data received from robot",null, 'stats'),
				states:_this.create_doorway("Code","Automatically drive the robot",help_text_states, 'list-alt'),
				map:_this.create_doorway("Map","See where the robot thinks it is",help_text_map, 'globe'),
				navigation:_this.create_doorway("Navigation","Plan a path for the robot",null, 'road'),
				video:_this.create_doorway("Video","Show the robot's video camera",null, 'facetime-video'),
				UI:_this.create_doorway("UI","Customized robot user interface",help_text_ui, 'object-align-top'),
				sound:_this.create_doorway("Sound","Play sounds on the backend to get attention",null, 'volume-up'),
				chat:_this.create_doorway("Chat","Chat with the caretaker of the robot.",null, 'comment')
			};

			clear_out(_this.doorways.config.content);
			clear_out(_this.doorways.pilot.content);
			clear_out(_this.doorways.sensors.content);
			clear_out(_this.doorways.charts.content);
			clear_out(_this.doorways.states.content);
			clear_out(_this.doorways.map.content);
			clear_out(_this.doorways.navigation.content);
			clear_out(_this.doorways.video.content);
			clear_out(_this.doorways.UI.content);
			clear_out(_this.doorways.sound.content);
			clear_out(_this.doorways.chat.content);

			_this.doorway_manager.hide_all();
			_this.doorways.config.set_minimized(false);

			_this.doorway_manager.load(json);
			_this.create_widgets();

			for(let key in _this.widgets)
				if(_this.widgets[key].download)
					_this.widgets[key].download(_this.robot);

			_this.gui.interval=setInterval(function(){_this.run_interval();},100);
		});

}

robot_ui_t.prototype.run_interval=function() {
	// Update sensor data
	var _this=this;

	if(!_this.robot.sim)
		_this.robot.sensors = robot_network.sensors;

	if (!_this.doorways.sensors.minimized)
		_this.widgets.sensors.refresh(JSON.parse(JSON.stringify(_this.robot.sensors)));

	if (!_this.doorways.map.minimized)
		_this.widgets.map.refresh(_this.robot.sensors);

	if(!_this.doorways.charts.minimized)
		_this.widgets.charts.refresh(_this.robot.sensors);

	_this.state_runner.VM_sensors=_this.robot.sensors;

	this.upload_gui();
}

robot_ui_t.prototype.upload_gui=function()
{
	var save=this.doorway_manager.save();
	var stringified=JSON.stringify(save);

	if(valid_robot(this.robot)&&this.gui.old!=stringified)
	{
		if(!this.robot.sim)
			superstar_set(this.robot,"gui",save);
		else
			sim_set(this.robot,"gui",save);
		this.gui.old=stringified;
	}
}

robot_ui_t.prototype.create_widgets=function()
{
	var _this=this;

	if(this.widgets)
		for(key in this.widgets)
			if(this.widgets[key].destroy)
				this.widgets[key].destroy();


	this.widgets=
	{
		config:new config_editor_t(this.doorways.config.content),
		states:new state_table_t(this.doorways.states,this.modal_div),
		pilot:new pilot_interface_t(this.doorways.pilot.content,this.doorways.pilot),
		charts:new chart_interface_t(this.doorways.charts.content),

		sensors:new tree_viewer_t(this.doorways.sensors.content,{},
		[
			{key:"bumper",type:"binary"}
		]),
		map:new robot_map_t(this.doorways.map.content,this.modal_div, _this.robot),
		navigation:new navigation_t(this.doorways.navigation.content, _this.state_runner, _this.robot),
		video:new video_widget_t(this.doorways.video,_this.pilot_heartbeat),
		UI:new UI_builder_t(this.doorways.UI.content),
		sound:new sound_player_t(this.doorways.sound.content,_this.robot),
		chat:new chatter_t(this.doorways.chat.content,_this.robot,20,"Pilot")
	};
	this.widgets.map.navigation=this.widgets.navigation;
	this.state_runner.set_UI(this.widgets.UI);

	this.widgets.config.onchange=function() { // recreate pilot GUI when configuration changes
		_this.widgets.pilot.reconfigure(_this.widgets.config);
	}

	this.widgets.config.onconfigure=function() // allow configuration upload
	{

		if(_this.robot&&_this.robot.name)
			_this.widgets.config.upload(_this.robot);
	}
	this.widgets.states.onrun=function()
	{
		setTimeout(function(){_this.doorways.UI.activate();},100);
		_this.widgets.UI.run();
		if(_this.robot.name)
		{
			_this.state_runner.VM_pilot=_this.widgets.pilot.pilot;
			_this.state_runner.run(_this.robot,_this.widgets.states);
		}
	}
	this.widgets.states.onstop=function()
	{
		if(_this.robot.name)
			_this.state_runner.stop(_this.widgets.states);
		_this.widgets.UI.stop();
	}
	this.state_runner.onpilot=function(power)
	{
		if(_this.robot.name)
			_this.widgets.pilot.upload(_this.robot);
	}

	if(this.doorways.map&&this.widgets.map)
	{
	var _this = this;
	this.doorways.map.resizer.onresize=function(){_this.widgets.map.resize_map()};
	}
	if(this.doorways.navigation&&this.widgets.navigation)
	{
		var _this = this;
		this.doorways.navigation.resizer.onresize=function(){_this.widgets.navigation.on_resize();};
		this.widgets.navigation.pilot=this.widgets.pilot.pilot;
	}
	
}

robot_ui_t.prototype.create_doorway=function(title,tooltip,help_text,icon)
{
	var doorway=this.doorway_manager.get_by_title(title);

	if(!help_text)
		help_text="";

	if(doorway)
		return doorway;
	else
		return this.doorway_manager.create(title,undefined,tooltip,help_text,icon);
}

robot_ui_t.prototype.request_uuid=function()
{
	var _this=this;
	var xmlhttp=new XMLHttpRequest();
	xmlhttp.onreadystatechange=function()
	{
		if(xmlhttp.readyState==4)
		{
			if(xmlhttp.status==200)
			{
				_this.uuid=xmlhttp.responseText;
				try
				{
					_this.widgets.video.uuid=_this.uuid;
				}
				catch(error)
				{
					var set_uuid=function()
					{
						try
						{
							_this.widgets.video.uuid=_this.uuid;
						}
						catch(error)
						{
							setTimeout(set_uuid,100);
						}
					};
					set_uuid();
				}
			}
			else
				setTimeout(function(){_this.request_uuid();},100);
		}
	};
	xmlhttp.open("GET","/superstar/?uuid=true&rand="+Date.now(),true);
	xmlhttp.send(null);
}
