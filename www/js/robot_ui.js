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

	this.menu=null;
	this.connect_menu=null;
	this.clone_menu=null;
	this.gui=
	{
		element:null,
		interval:null,
		old:""
	};
	this.doorways={}
	this.widgets={};

	this.create_gui();
	this.connect_menu.show();
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
		"Robot",
		null,
		"glyphicon glyphicon-cog",
		{
			"Connect":
			{
				onclick:function(){myself.connect_menu.show();},
				glyph:"glyphicon glyphicon-off"
			},
			"Clone":
			{
				onclick:function(){myself.clone_menu.show();},
				glyph:"glyphicon glyphicon-duplicate"
			}
		}
	);
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
		myself.gui.element.load(json);
		myself.create_widgets();
		myself.clone_menu.clone_target=myself.robot_name;

		for(var key in myself.widgets)
			if(myself.widgets[key].download)
				myself.widgets[key].download(myself.robot_name);

		myself.gui.interval=setInterval(function(){myself.run_interval();},1000);
	});
}


robot_ui_t.prototype.run_interval=function() {
	// Update sensor data 
	var myself=this;
	superstar_get(this.robot_name,"sensors",
		function(sensor_data)
		{
			myself.widgets.sensor.refresh(sensor_data);
		});

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

	this.doorways=
	{
		config:this.create_doorway("Configure"),
		pilot:this.create_doorway("Drive"),
		sensor:this.create_doorway("Sensor"),
		states:this.create_doorway("Code")
	};

	this.widgets=
	{
		config:new config_editor_t(this.doorways.config.content,this.robot_name),
		states:new state_table_t(this.doorways.states.content),
		pilot:new pilot_interface_t(this.doorways.pilot.content),
		sensor:new tree_viewer_t(this.doorways.sensor.content,{})
	};

	this.widgets.config.onconfigure=function()
	{
		if(myself.robot_name)
			myself.widgets.config.upload(myself.robot_name);
	}
	this.widgets.states.onrun=function()
	{
		if(myself.robot_name)
			myself.widgets.states.upload(myself.robot_name);
	}
	this.widgets.pilot.onpilot=function(power)
	{
		console.log("Pilot data upload: "+myself.robot_name);
		if(myself.robot_name)
			myself.widgets.pilot.upload(myself.robot_name);
	}
}

robot_ui_t.prototype.create_doorway=function(title)
{
	var doorway=this.gui.element.get_by_title(title);

	if(doorway)
		return doorway;
	else
		return this.gui.element.create(title);
}
