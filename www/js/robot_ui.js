/**
  This is the new 2015-07 modular robot interface, with a row of buttons on top,
  and moveable "doorways" showing each robot interface element.
*/

//to add a widget, add an entry to create_widgets.
//if the widget has a download method, it will be called.
//if the widget has a upload method, add the callback in create_widgets.

function robot_ui_t(div)
{
	if(!div)
		return null;

	this.div=div;
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
		element:null,
		interval:null,
		old:""
	};
	this.sensor_data_count=0;
	this.doorways={};
	this.widgets={};

	this.create_gui();

	var myself=this;
	var options=parse_uri(location.search);
	validate_robot_name(options.robot,
		// good robot:
		function(robot)
		{
			//console.log("Connecting to URL robot "+JSON.stringify(robot));
			myself.connect_menu.onconnect(robot);
		},
		// bad robot:
		function()
		{
			myself.connect_menu.show();
		});
	this.pilot_heartbeat = new pilot_status_t(this);
}

robot_ui_t.prototype.create_menus=function()
{
	var myself=this;

	this.menu=new robot_menu_t(div);
	this.connect_menu=new modal_connect_t(div);

	this.menu.get_status_area().innerHTML=this.disconnected_text;
	this.menu.create_button
	(
		"Connect",
		function(){myself.connect_menu.show();},//null,
		"glyphicon glyphicon-off",//"glyphicon glyphicon-cog",
		null,
		"Connect to a new robot over the network"
	);

	this.connect_menu.onconnect=function(robot)
	{
		
		if(robot)
		{
			if(robot.sim) myself.robot=robot;
			else 
			{
				myself.robot=JSON.parse(JSON.stringify(robot));
				robot_network.sim="";
			}
			clearInterval(myself.gui.interval);
			myself.gui.interval=null;
			myself.menu.get_status_area().innerHTML="Connected to \""+
				myself.robot.year+"/"+myself.robot.school+"/"+myself.robot.name+"\"";
			myself.download_gui();
			//myself.widgets.chat.set_robot(robot);
			robot_network.set_robot(robot);
		}
	};
}


robot_ui_t.prototype.create_gui=function()
{
	this.create_menus();
	this.gui.element=new doorways_t(div,this.menu.get_menu_bar());
}

robot_ui_t.prototype.download_gui=function()
{
	if(!valid_robot(this.robot))
		return;

	var myself=this;

	var help_text_states =
	//"<h3> Code Examples </h3>"
	"<h4>Basic</h4>"
	+ "print(\"This will appear to the right of the state\") <br>"
	+ "stop() <br>"
	+ "state = \"turnLeft\" // <i>Runs your state named \"turnLeft\"</i><br>"
	+ "<h4>UI</h4>"
	+ "label(\"This will appear in the UI\")<br>"
	+ "button(\"Press me\", \"turnLeft\")<br>"
	+ "power.L=slider(\"left wheel\", -1.0, 0.0, 1.0) <br>"
	+ "checkbox(\"Would you like to check this box?\")<br>"
	//+ "if (checkbox(\"Would you like to check this box?\")) { print(\"Yes!\") } <br>"
	+ "<h4>Drive</h4>"
	+ "forward(10)  // <i>drive forward 10 centimeters</i><br>"
	+ "backward(5)  // <i>drive backward 5 centimeters</i><br>"
	+ "left(45)  // <i>turn left 45 degrees</i><br>"
	+ "right(90)  // <i>turn right 90 degrees</i><br>"
	+ "drive(-0.5, 0.5)  // <i>Sets the speed of the left and right wheels</i><br>"
	+ "<h4>Servos</h4>"
	+ "power.servo[0] = 90 // <i>Moves the first servo to position 90 degrees (range is 0 to 180)"
	+ "<br><a href=\"/code/api/\" title=\"API\" target=\"new\"> <h4>Advanced</h4> </a>";
	//+ "<h4> Sensors </h4>"
	//+ "sensors.location.x - <i> Robot location in x-coordinate</i><br>"
	//+ "sensors.location.angle - <i> Direction robot is facing </i><br>"
	//+ "sensors.floor[2] - <i> Floor sensor 2 </i><br>";

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

	var help_text_ui =
	"Run your Code to add elements to the UI <br>"
	+ "<h4>UI elements:</h4>"
	+ "label(\"This will appear in the UI\")<br>"
	+ "button(\"Press me\", state1)<br>"
	+ "slider(\"Slider!\", -1.0, 0.0, 1.0) <br>"
	+ "checkbox(\"Would you like to check this box?\")<br>";

	var help_text_config =
	"<h3>When configuring devices:</h3>"
	+"<h4><b>wheel_encoder</b></h4>"
	+"<ul><li>Pin: Left wheel encoder pin</li><li>Pin: Right wheel encoder pin</li><li>"
	+"Number: Robot wheelbase <ul><li>Distance between wheel centers (mm)</li></ul></li></ul>";

	var clear_out=function(div)
	{
		while(div.firstChild)
			div.removeChild(div.firstChild);
	}

	superstar_get(this.robot,"gui",function(json)
	{
		myself.doorways=
		{
			config:myself.create_doorway("Configure","Set up robot hardware",help_text_config),
			pilot:myself.create_doorway("Drive","Manually drive the robot",null),
			sensors:myself.create_doorway("Sensors","Examine sensor data from robot",help_text_sensors),
			charts:myself.create_doorway("Charts", "Chart sensor data received from robot",null),
			states:myself.create_doorway("Code","Automatically drive the robot",help_text_states),
			map:myself.create_doorway("Map","See where the robot thinks it is",null),
			video:myself.create_doorway("Video","Show the robot's video camera",null),
			UI:myself.create_doorway("UI","Customized robot user interface",help_text_ui),
			sound:myself.create_doorway("Sound","Play sounds on the backend to get attention",null),
			chat:myself.create_doorway("Chat","Chat with the caretaker of the robot.",null)
		};

		clear_out(myself.doorways.config.content);
		clear_out(myself.doorways.pilot.content);
		clear_out(myself.doorways.sensors.content);
		clear_out(myself.doorways.charts.content);
		clear_out(myself.doorways.states.content);
		clear_out(myself.doorways.map.content);
		clear_out(myself.doorways.video.content);
		clear_out(myself.doorways.UI.content);
		clear_out(myself.doorways.sound.content);
		clear_out(myself.doorways.chat.content);

		myself.gui.element.hide_all();
		myself.gui.element.minimize(myself.doorways.config,false);

		myself.gui.element.load(json);
		myself.create_widgets();

		for(var key in myself.widgets)
			if(myself.widgets[key].download)
				myself.widgets[key].download(myself.robot);

		myself.gui.interval=setInterval(function(){myself.run_interval();},100);
	});
}

robot_ui_t.prototype.run_interval=function() {
	// Update sensor data
	var myself=this;

	if (myself.sensor_data_count<2)
	{ // request more sensor data
		this.sensor_data_count++;
		//superstar_get(this.robot,"sensors",
			//function(sensors) // sensor data has arrived:
			//{
				myself.sensor_data_count--;
				if(!myself.robot.sim)
				{
					if (!myself.doorways.sensors.minimized)
						{
						//console.log("refreshing sensors from robot network: " + JSON.stringify(robot_network.sensors));
						myself.widgets.sensors.refresh(robot_network.sensors);
						}	

					if (!myself.doorways.map.minimized)
						myself.widgets.map.refresh(robot_network.sensors);

					if(!myself.doorways.charts.minimized)
						myself.widgets.charts.refresh(robot_network.sensors);

					myself.state_runner.VM_sensors=robot_network.sensors;
				}
				else
				{
					
					if (!myself.doorways.sensors.minimized)
					{	var sensors_json = JSON.parse(JSON.stringify(myself.robot.sensors));
						//console.log("refreshing sensors from simulation: " + JSON.stringify(myself.robot.sensors));
						myself.widgets.sensors.refresh(sensors_json);
					}

					if (!myself.doorways.map.minimized)
						myself.widgets.map.refresh(myself.robot.sensors);

					if(!myself.doorways.charts.minimized)
						myself.widgets.charts.refresh(myself.robot.sensors);

					myself.state_runner.VM_sensors=myself.robot.sensors;
				}
			//});
	}

	this.upload_gui();
}

robot_ui_t.prototype.upload_gui=function()
{
	var save=this.gui.element.save();
	var stringified=JSON.stringify(save);

	if(valid_robot(this.robot)&&this.gui.old!=stringified)
	{
		superstar_set(this.robot,"gui",save);
		this.gui.old=stringified;
	}
}

robot_ui_t.prototype.create_widgets=function()
{
	var myself=this;

	if(this.widgets)
		for(key in this.widgets)
			if(this.widgets[key].destroy)
				this.widgets[key].destroy();

		
	this.widgets=
	{
		config:new config_editor_t(this.doorways.config.content),
		states:new state_table_t(this.doorways.states),
		pilot:new pilot_interface_t(this.doorways.pilot.content),
		charts:new chart_interface_t(this.doorways.charts.content),
		
		sensors:new tree_viewer_t(this.doorways.sensors.content,{},
		[
			{key:"bumper",type:"binary"}
		]),
		map:new robot_map_t(this.doorways.map.content,{}),
		video:new video_widget_t(this.doorways.video,myself.pilot_heartbeat),
		UI:new UI_builder_t(this.doorways.UI.content),
		sound:new sound_player_t(this.doorways.sound.content,myself.robot),
		chat:new chatter_t(this.doorways.chat.content,myself.robot,20,"Pilot")
	};
	this.state_runner.set_UI(this.widgets.UI);

/* // This blanks out the map entirely when it's not on top--bad way to save CPU!
	this.doorways.map.ondeactivate=function()
	{
		if(myself.widgets.map.renderer)
			myself.widgets.map.renderer.show(false);
	};
	this.doorways.map.onactivate=function()
	{
		if(myself.widgets.map.renderer)
			myself.widgets.map.renderer.show(true);
	};
*/

	this.widgets.config.onchange=function() { // recreate pilot GUI when configuration changes
		myself.widgets.pilot.reconfigure(myself.widgets.config);
	}

	this.widgets.config.onconfigure=function() // allow configuration upload
	{
		
		if(myself.robot&&myself.robot.name)
			myself.widgets.config.upload(myself.robot);
	}
	this.widgets.states.onrun=function()
	{
		setTimeout(function(){myself.gui.element.activate(myself.doorways.UI);},100);
		myself.widgets.UI.run();
		if(myself.robot.name)
		{
			myself.state_runner.VM_pilot=myself.widgets.pilot.pilot;
			myself.state_runner.run(myself.robot,myself.widgets.states);
		}
	}
	this.widgets.states.onstop=function()
	{
		if(myself.robot.name)
			myself.state_runner.stop(myself.widgets.states);
		myself.widgets.UI.stop();
	}
	this.widgets.pilot.onpilot=myself.state_runner.onpilot=function(power)
	{
		//console.log("Pilot data upload: "+myself.robot.name);
		if(myself.robot.name)
			myself.widgets.pilot.upload(myself.robot);
	}
	
	if(this.doorways.map&&this.widgets.map)
	{
	var myself = this;
	this.doorways.map.resizer.onresize=function(){myself.widgets.map.resize_map()};
	}
}

robot_ui_t.prototype.create_doorway=function(title,tooltip,help_text)
{
	var doorway=this.gui.element.get_by_title(title);

	if(!help_text)
		help_text="";

	if(doorway)
		return doorway;
	else
		return this.gui.element.create(title,undefined,tooltip,help_text);
}
