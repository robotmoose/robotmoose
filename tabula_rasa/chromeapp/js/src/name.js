/**
  List schools and robot names from superstar in
  a drop-down menu for the user to select.

  on_message(message) - Callback called when something should be added to a log.
  on_selected(robot) - Callback called when a valid school and robot combination is chosen.
*/

function name_t(div,on_message,on_selected)
{
	if(!div)
		return null;

	this.inital_startup=false;
	this.div=div;
	this.el=new_div(this.div);

	this.on_message=on_message;
	this.on_selected=on_selected;
	this.on_loaded_robot={};
	this.disabled=false;

	var _this=this;

	this.superstar_ok=false;
	this.superstar_checkmark=new checkmark_t(this.el);
	this.superstar_select=new dropdown_t(this.superstar_checkmark.getElement());
	this.superstar_select.el.style.width="100%";
	this.superstar_select.select.style.width="100%";
	this.superstar_select.onchange=function(){_this.download_years_m();};

	this.name_checkmark=new checkmark_t(this.el);
	this.year_select=new dropdown_t(this.name_checkmark.getElement());
	this.year_select.set_width("34%");
	this.year_select.onchange=function(){_this.download_schools_m();};
	this.school_select=new dropdown_t(this.name_checkmark.getElement());
	this.school_select.set_width("33%");
	this.school_select.onchange=function(){_this.download_robots_m();};
	this.robot_select=new dropdown_t(this.name_checkmark.getElement());
	this.robot_select.set_width("33%");
	this.robot_select.onchange=function(){_this.on_selected_m();};

	this.disables_interval=setInterval(function()
	{
		_this.update_disables_m();
	},250);
}


name_t.prototype.destroy=function()
{
	clearInterval(this.disables_interval);
	clearInterval(this.school_interval);
	this.div.removeChild(this.el);
}

name_t.prototype.get_robot=function()
{
	var robot={superstar:"robotmoose.com",school:null,name:null,year:null};

	if(this.year_select.selected_index()>0&&
		this.school_select.selected_index()>0&&
		this.robot_select.selected_index()>0)
	{
		robot.year=this.year_select.selected();
		robot.school=this.school_select.selected();
		robot.name=this.robot_select.selected();
		robot.superstar=this.superstar_select.selected();
	}

	return robot;
}

name_t.prototype.load=function(robot)
{
	this.onloaded_robot=robot;
	if(!this.onloaded_robot)
		this.onloaded_robot={};
	if(!this.onloaded_robot.superstar)
		this.onloaded_robot.superstar="robotmoose.com";
	this.update_superstars_m();
}

name_t.prototype.build_select_m=function(select,json,heading,on_loaded_value,sort_func)
{
	select.length=0;
	var list=[];
	list.push(heading);
	for(let key in json)
		list.push(json[key]);
	if(sort_func)
		list.sort(sort_func);
	select.build(list,on_loaded_value);
	this.update_disables_m();
}

name_t.prototype.update_superstars_m=function()
{
	var superstar_options=
	[
		"robotmoose.com",
		"test.robotmoose.com",
		"127.0.0.1:8081",
		"10.8.110.36:8081"
	];

	this.build_superstars_m(superstar_options);
	this.download_years_m();
}

name_t.prototype.update_years_m=function(json)
{
	this.build_years_m(json);
	this.download_schools_m();
}

name_t.prototype.update_schools_m=function(json)
{
	this.build_schools_m(json);
	this.download_robots_m();
}

name_t.prototype.update_robots_m=function(json)
{
	this.build_robots_m(json);
	if(this.onloaded_robot)
	{
		this.onloaded_robot={};
		this.on_selected_m();
	}
}

name_t.prototype.build_superstars_m=function(json)
{
	var value=this.school_select.selected();
	if(this.onloaded_robot.superstar)
		value=this.onloaded_robot.superstar;
	this.build_select_m(this.superstar_select,json,"Superstar",value);
}

name_t.prototype.build_years_m=function(json)
{
	var value=this.year_select.selected();
	if(this.onloaded_robot.year)
		value=this.onloaded_robot.year;
	this.build_select_m(this.year_select,json,"Year",value,function(lhs,rhs)
	{
		return parseInt(rhs)-parseInt(lhs);
	});
}

name_t.prototype.build_schools_m=function(json)
{
	var value=this.school_select.selected();
	if(this.onloaded_robot.school)
		value=this.onloaded_robot.school;
		this.build_select_m(this.school_select,json,"School",value);
}

name_t.prototype.build_robots_m=function(json)
{
	var value=this.robot_select.selected();
	if(this.onloaded_robot.name)
		value=this.onloaded_robot.name;
	this.build_select_m(this.robot_select,json,"Robot",value);
}

name_t.prototype.on_error_m=function(error)
{
	if(this.on_message)
		this.on_message(error);
}

name_t.prototype.download_years_m=function()
{
	robot_set_superstar(this.superstar_select.selected());

	if(this.superstar_select.selected_index()>0)
	{
		var _this=this;
		superstar_sub
		(
			{
				superstar:this.superstar_select.selected(),
				year:"",
				school:"",
				name:""
			},
			"/",
			function(json)
			{
				_this.update_years_m(json);
				_this.superstar_ok=true;
			},
			function(error)
			{
				_this.on_error_m("Year download error("+error.code+") "+error.message);
				_this.superstar_ok=false;
			}
		);
		superstar.flush();
		return;
	}
	this.update_years_m();
}

name_t.prototype.download_schools_m=function()
{
	if(this.superstar_select.selected_index()>0&&
		this.year_select.selected_index()>0)
	{
		var _this=this;
		superstar_sub
		(
			{
				superstar:this.superstar_select.selected(),
				year:this.year_select.selected(),
				school:"",
				name:""
			},
			"/",
			function(json)
			{
				_this.update_schools_m(json);
				_this.superstar_ok=true;
			},
			function(error)
			{
				_this.on_error_m("School download error("+error.code+") "+error.message);
				_this.superstar_ok=false;
			}
		);
		superstar.flush();
		return;
	}
	this.update_schools_m();
}

name_t.prototype.download_robots_m=function()
{
	if(this.superstar_select.selected_index()>0&&
		this.year_select.selected_index()>0&&
		this.school_select.selected_index()>0)
	{
		var _this=this;
		superstar_sub
		(
			{
				superstar:this.superstar_select.selected(),
				year:this.year_select.selected(),
				school:this.school_select.selected(),
				name:""
			},
			"/",
			function(json)
			{
				_this.update_robots_m(json);
				_this.superstar_ok=true;
			},
			function(error)
			{
				_this.on_error_m("Robot download error("+error.code+") "+error.message);
				_this.superstar_ok=false;
			}
		);
		superstar.flush();
		return;
	}
	this.update_robots_m();
}

name_t.prototype.update_disables_m=function()
{
	this.superstar_select.set_enable(!this.disabled);
	this.year_select.set_enable(!this.disabled);
	this.school_select.set_enable(!this.disabled);
	this.robot_select.set_enable(!this.disabled);
	this.superstar_checkmark.check(this.superstar_select.selected_index()>0&&
		this.superstar_ok);
	this.name_checkmark.check(this.year_select.selected_index()>0&&
		this.school_select.selected_index()>0&&
		this.robot_select.selected_index()>0);
}

name_t.prototype.on_selected_m=function(robot)
{
	if(!robot)
		var robot=this.get_robot();
	if(this.on_selected&&robot.school!=null&&robot.name!=null)
		this.on_selected(robot);
}
