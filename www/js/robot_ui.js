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
		name:null,
		auth:null
	};
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

	var options=parse_uri(location.search);
	var myself=this;

	validate_robot_name(options.robot,
		function()
		{
			myself.robot.name=options.robot;
			myself.connect_menu.onconnect(myself.robot.name,myself.robot.auth);
		},
		function()
		{
			myself.connect_menu.show();
		});
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

	this.connect_menu.onconnect=function(robot_name,robot_auth)
	{
		if(robot_name)
		{
			myself.robot.name=null;
			clearInterval(myself.gui.interval);
			myself.gui.interval=null;
			myself.robot.name=robot_name;
			myself.robot.auth=robot_auth;
			myself.menu.get_status_area().innerHTML="Connected to \""+myself.robot.name+"\"";

			myself.download_gui();
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
	if(!this.robot||!this.robot.name)
		return;

	var myself=this;
	
	var help_text_states = 
	//"<h3> Code Examples </h3>"
	"<h4>Basic</h4>"
	+ "print(\"This will appear to the right of the state\") <br>"
	+ "stop() <br>"
	+ "state = state1 // <i>Runs the state named \"state1\"</i><br>"
	+ "<h4>UI</h4>"
	+ "label(\"This will appear in the UI\")<br>"
	+ "button(\"Press me\", state1)<br>"
	+ "slider(\"Slider!\", -1.0, 0.0, 1.0) <br>"
	//+ "print(slider(\"Slider!\", -1.0, 0.0, 1.0)) <br>"
	+ "checkbox(\"Would you like to check this box?\")<br>"
	//+ "if (checkbox(\"Would you like to check this box?\")) { print(\"Yes!\") } <br>"
	+ "<h4>Drive</h4>"
	+ "drive(-0.5, 0.5)  // <i>Sets the speed of the left and right wheels</i><br>"
	+ "<br><a href=\"/lessons/ui/\" title=\"UI Examples\"> <h4>Advanced</h4> </a>";
	//+ "<h4> Sensors </h4>"
	//+ "sensors.location.x - <i> Robot location in x-coordinate</i><br>"
	//+ "sensors.location.angle - <i> Direction robot is facing </i><br>"
	//+ "sensors.floor[2] - <i> Floor sensor 2 </i><br>";

	var clear_out=function(div)
	{
		while(div.firstChild)
			div.removeChild(div.firstChild);
	}

	superstar_get(this.robot.name,"gui",function(json)
	{
		myself.doorways=
		{
			config:myself.create_doorway("Configure","Set up robot hardware",null),
			pilot:myself.create_doorway("Drive","Manually drive the robot",null),
			sensors:myself.create_doorway("Sensors","Examine sensor data from robot",null),
			states:myself.create_doorway("Code","Automatically drive the robot",help_text_states),
			map:myself.create_doorway("Map","See where the robot thinks it is",null),
			video:myself.create_doorway("Video","Show the robot's video camera",null),
			UI:myself.create_doorway("UI","Customized robot user interface",null)
		};

		clear_out(myself.doorways.config.content);
		clear_out(myself.doorways.pilot.content);
		clear_out(myself.doorways.sensors.content);
		clear_out(myself.doorways.states.content);
		clear_out(myself.doorways.map.content);
		clear_out(myself.doorways.video.content);
		clear_out(myself.doorways.UI.content);

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
		superstar_get(this.robot.name,"sensors",
			function(sensors) // sensor data has arrived:
			{
				myself.sensor_data_count--;
				
				if (!myself.doorways.sensors.minimized)
					myself.widgets.sensors.refresh(sensors);
				
				if (!myself.doorways.map.minimized)
					myself.widgets.map.refresh(sensors);

				myself.state_runner.VM_sensors=sensors;
			});
	}

	this.upload_gui();
}

robot_ui_t.prototype.upload_gui=function()
{
	var save=this.gui.element.save();
	var stringified=JSON.stringify(save);

	if(this.robot&&this.robot.name&&this.gui.old!=stringified)
	{
		superstar_set(this.robot.name,"gui",save,null,this.robot.auth);
		this.gui.old=stringified;
	}
}

robot_ui_t.prototype.create_widgets=function()
{
	var myself=this;

	this.widgets=
	{
		config:new config_editor_t(this.doorways.config.content),
		states:new state_table_t(this.doorways.states),
		pilot:new pilot_interface_t(this.doorways.pilot.content),
		sensors:new tree_viewer_t(this.doorways.sensors.content,{}),
		map:new robot_map_t(this.doorways.map.content,{}),
		video:new video_widget_t(this.doorways.video),
		UI:new UI_builder_t(this.doorways.UI.content)
	};
	this.state_runner.set_UI(this.widgets.UI);

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
		console.log("Pilot data upload: "+myself.robot.name);
		if(myself.robot.name)
			myself.widgets.pilot.upload(myself.robot);
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
