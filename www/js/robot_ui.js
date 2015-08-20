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
	this.cloned=false;
	this.robot_name=null;
	this.disconnected_text="<font style='color:red;'>Not connected.</font>";

	this.state_runner=new state_runner_t();
	this.menu=null;
	this.connect_menu=null;
	this.clone_menu=null;
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

	if(options.robot)
	{
		this.robot_name=options.robot;
		this.connect_menu.onconnect(this.robot_name);
	}
	else
	{
		this.connect_menu.show();
	}
}

robot_ui_t.prototype.clone=function(to,from,setting)
{
	var myself=this;
	superstar_get(from,setting,function(obj)
	{
		superstar_set(to,setting,obj,function(obj){myself.cloned=true;});
	});
}

robot_ui_t.prototype.clone_reconnect=function(robot_name)
{
	var myself=this;

	if(this.cloned)
		this.connect_menu.onconnect(robot_name);
	else
		setTimeout(function(){myself.clone_reconnect(robot_name);},50);
}

robot_ui_t.prototype.create_menus=function()
{
	var myself=this;

	this.menu=new robot_menu_t(div);
	this.connect_menu=new modal_connect_t(div);
	this.clone_menu=new modal_clone_t(div);

	this.menu.get_status_area().innerHTML=this.disconnected_text;
	this.menu.create_button
	(
		"Connect",
		function(){myself.connect_menu.show();},//null,
		"glyphicon glyphicon-off",//"glyphicon glyphicon-cog",
		null,
		"Connect to a new robot over the network"
	);

	if(this.menu.buttons["Robot"]&&this.menu.buttons["Robot"].drops["Clone"])
		this.menu.buttons["Robot"].drops["Clone"].disable();

	this.connect_menu.onconnect=function(robot_name)
	{
		if(robot_name)
		{
			myself.robot_name=null;
			clearInterval(myself.gui.interval);
			myself.gui.interval=null;
			myself.robot_name=robot_name;
			myself.menu.get_status_area().innerHTML="Connected to \""+myself.robot_name+"\"";

			if(myself.menu.buttons["Robot"]&&myself.menu.buttons["Robot"].drops["Clone"])
				myself.menu.buttons["Robot"].drops["Clone"].enable();

			myself.download_gui();
		}
	};
	myself.clone_menu.clone_target=myself.robot_name;
	myself.clone_menu.onclone=function(robot_name,settings,options)
	{
		if(myself.clone_menu.clone_target)
		{
			for(var key in settings)
				myself.clone(robot_name,myself.clone_menu.clone_target,settings[key]);

			var connect_to_clone=false;

			for(var key in options)
			{
				if(options[key]=="connect_to_clone")
					myself.connect_to_clone=true;
			}

			if(connect_to_clone)
				myself.clone_reconnect(robot_name);
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
	if(!this.robot_name)
		return;

	var myself=this;

	superstar_get(this.robot_name,"gui",function(json)
	{
		myself.doorways=
		{
			config:myself.create_doorway("Configure","Set up robot hardware"),
			pilot:myself.create_doorway("Drive","Manually drive the robot"),
			sensors:myself.create_doorway("Sensors","Examine sensor data from robot"),
			states:myself.create_doorway("Code","Automatically drive the robot"),
			map:myself.create_doorway("Map","See where the robot thinks it is"),
			video:myself.create_doorway("Video","Show the robot's video camera")
		};
		myself.gui.element.hide_all();
		myself.gui.element.minimize(myself.doorways.config,false);

		myself.gui.element.load(json);
		myself.create_widgets();
		myself.clone_menu.clone_target=myself.robot_name;

		for(var key in myself.widgets)
			if(myself.widgets[key].download)
				myself.widgets[key].download(myself.robot_name);

		myself.gui.interval=setInterval(function(){myself.run_interval();},100);
	});
}

robot_ui_t.prototype.run_interval=function() {
	// Update sensor data
	var myself=this;

	if (myself.sensor_data_count<2)
	{ // request more sensor data
		this.sensor_data_count++;
		superstar_get(this.robot_name,"sensors",
			function(sensors) // sensor data has arrived:
			{
				myself.sensor_data_count--;
				myself.widgets.sensors.refresh(sensors);
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

	if(this.robot_name&&this.gui.old!=stringified)
	{
		superstar_set(this.robot_name,"gui",save);
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
		video:new video_gruveo_t(this.doorways.video.content)
	};

	this.widgets.config.onchange=function() { // recreate pilot GUI when configuration changes
		myself.widgets.pilot.reconfigure(myself.widgets.config);
	}

	this.widgets.config.onconfigure=function() // allow configuration upload
	{
		if(myself.robot_name)
			myself.widgets.config.upload(myself.robot_name);
	}
	this.widgets.states.onrun=function()
	{
		if(myself.robot_name)
		{
			myself.state_runner.VM_power=myself.widgets.pilot.pilot.power;
			myself.widgets.states.upload(myself.robot_name);
			myself.state_runner.run(myself.widgets.states);
		}
	}
	this.widgets.states.onstop=function()
	{
		if(myself.robot_name)
			myself.state_runner.stop(myself.widgets.states);
	}
	this.widgets.pilot.onpilot=myself.state_runner.onpilot=function(power)
	{
		console.log("Pilot data upload: "+myself.robot_name);
		if(myself.robot_name)
			myself.widgets.pilot.upload(myself.robot_name);
	}
}

robot_ui_t.prototype.create_doorway=function(title,tooltip)
{
	var doorway=this.gui.element.get_by_title(title);

	if(doorway)
		return doorway;
	else
		return this.gui.element.create(title,undefined,tooltip);
}
