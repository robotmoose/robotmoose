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

	this.div=div;
	this.el=new_div(this.div);

	this.on_message=on_message;
	this.on_selected=on_selected;
	this.on_loaded_robot={};
	this.disabled=false;

	var _this=this;

	this.superstar_div=document.createElement("div");
	this.superstar_select=new dropdown_t(this.superstar_div);
	this.el.appendChild(this.superstar_div);
	this.superstar_select.el.style.width="100%";
	this.superstar_select.select.style.width="100%";
	this.superstar_select.onchange=function()
	{
		if(this.selectedIndex!=0)
		{
			_this.build_years_m();
			_this.build_schools_m();
			_this.build_robots_m();
		}
		else
		{
			_this.build_select_m(_this.school_select,[],"School","School");
			_this.build_select_m(_this.robot_select,[],"Robot","Robot");
			_this.build_select_m(_this.year_select,[],"Year","Year");
		}
	};

	this.year_school_robot_div=document.createElement("div");
	this.year_school_robot_div.style.width="100%";

	this.year_select=new dropdown_t(this.year_school_robot_div);
	this.year_select.set_width("33%");

	this.school_select=new dropdown_t(this.year_school_robot_div);
	this.school_select.set_width("33%");
	this.school_select.onchange=function(){_this.download_robots_m();};

	this.robot_select=new dropdown_t(this.year_school_robot_div);
	this.robot_select.set_width("33%");
	this.robot_select.onchange=function(){_this.on_selected_m();};

	this.el.appendChild(this.year_school_robot_div);


	_this.school_interval=setInterval(function()
	{
		if(!_this.superstar_select.disabled)
		{
				_this.on_loaded_robot=
				{
					superstar:_this.superstar_select.selected(),
					year: _this.year_select.selected(),
					school:_this.school_select.selected(),
					name:_this.robot_select.selected()
				};
				_this.download_years_m();
		}
	},250);

	_this.build_superstar_m();
	_this.build_years_m();
	_this.build_schools_m();
	_this.build_robots_m();

	_this.disables_interval=setInterval(function(){_this.update_disables_m();},250);
	_this.download_years_m();
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

	if(this.year_select.selected_index()>0
			&&this.school_select.selected_index()>0
			&&this.robot_select.selected_index()>0)
	{
		robot.year=this.year_select.selected();
		robot.school=this.school_select.selected();
		robot.name=this.robot_select.selected();
		robot.superstar=this.superstar_select.selected();
	}

	return robot;
}

name_t.prototype.reload=function(robot)
{
	var need_reload=(!this.on_loaded_robot.name||!this.on_loaded_robot.school);
	this.on_loaded_robot=robot;
	this.build_superstar_m([]);
	this.build_years_m([]);
	this.build_schools_m([]);
	this.build_robots_m([]);

	if(need_reload)
		this.on_selected_m(this.on_loaded_robot);
}

name_t.prototype.build_select_m=function(select,json,heading,on_loaded_value)
{
	select.length=0;
	var list=[];

	list.push(heading);
	for(var key in json)
		list.push(json[key]);
	select.build(list,on_loaded_value);
	this.update_disables_m();
}

name_t.prototype.build_years_m=function(json)
{
	this.build_select_m(this.year_select, json, "Year", this.on_loaded_robot.year);
	this.download_schools_m();
}

name_t.prototype.build_schools_m=function(json)
{
	this.build_select_m(this.school_select,json,"School",this.on_loaded_robot.school);
	this.download_robots_m();
}

name_t.prototype.build_robots_m=function(json)
{
	this.build_select_m(this.robot_select,json,"Robot",this.on_loaded_robot.name);
}

name_t.prototype.build_superstar_m=function()
{
	var superstar_options=
	[
		"robotmoose.com",
		"test.robotmoose.com",
		"127.0.0.1:8081"
	];

	if(!this.on_loaded_robot.superstar)
		this.on_loaded_robot.superstar=superstar_options[0];

	this.build_select_m(this.superstar_select,superstar_options,"Superstar",this.on_loaded_robot.superstar);
}

name_t.prototype.on_error_m=function(error)
{
	if(this.on_message)
		this.on_message(error);
}

name_t.prototype.download_years_m=function()
{
	if(this.superstar_select.selected() && this.superstar_select.selected_index()>0){
		var _this=this;
		superstar_sub(
			{
				superstar:this.on_loaded_robot.superstar,
				year:"",
				school:"",
				name:""
			},
			"/",
			function(json){_this.build_years_m(json);},
			function(error){
				_this.on_error_m("Year download error("+error+").");
			}
		);
	}
}

name_t.prototype.download_schools_m=function()
{
	var selected_year=this.year_select.selected();
	if(!selected_year || this.year_select.selected_index<=0)
	{
		this.build_schools_m();
		return;
	}


	if(this.superstar_select.selected() && this.superstar_select.selected_index()>0
			&& selected_year)
	{
		var _this=this;
		superstar_sub(
			{
				superstar:this.on_loaded_robot.superstar,
				school:"",
				name:"",
				year: selected_year
			},
			"/",
			function(json){ _this.build_schools_m(json); },
			function(error){
				_this.on_error_m("School download error ("+error+").");
			}
		);
		return;
	}
	this.build_schools_m([]);
	this.build_robots_m([]);
}

name_t.prototype.download_robots_m=function()
{
	if(!this.school_select.selected() || this.school_select.selected_index<=0)
	{
		this.build_robots_m();
		return;
	}

	var selected_year=this.year_select.selected();
	var selected_school=this.school_select.selected();

	if(this.superstar_select.selected() && this.superstar_select.selected_index()>0
			&& this.year_select.selected_index() >0 && this.school_select.selected_index() > 0
			&& selected_school && selected_year)
	{
		var _this=this;

		superstar_sub(
			{
				superstar:this.superstar_select.selected(),
				school:selected_school,
				name:"",
				year:selected_year
			},
			"/",
			function(json){_this.build_robots_m(json);},
			function(error){_this.on_error_m("Robots download error ("+error+").");});
	}
}

name_t.prototype.update_disables_m=function()
{
	if(this.superstar_select.selected_index()==0){
		this.superstar_select.set_background_color("maroon");
		this.year_select.disabled=true;
	}
	else{
		this.superstar_select.set_background_color("cyan");
		this.year_select.disabled=false;
	}
	if(this.year_select.selected_index()==0 || this.superstar_select.disabled){
		this.year_select.set_background_color("maroon");
		this.school_select.disabled=true;
	}
	else{
		this.year_select.set_background_color("cyan");
		this.school_select.disabled=false;
	}
	if(this.school_select.selected_index()==0 || this.year_select.disabled){
		this.school_select.set_background_color("maroon");
		this.robot_select.disabled=true;
	}
	else{
		this.school_select.set_background_color("cyan");
		this.robot_select.disabled=false;
	}
	if(this.robot_select.selected_index()==0 || this.school_select.disabled){
		this.robot_select.set_background_color("maroon");
	}
	else{
		this.robot_select.set_background_color("cyan");
	}

	/*
	var disabled=false;
	if(this.school_select.selected_index<=0||this.disabled)
		disabled=true;
	this.robot_select.disabled=disabled;

	this.school_select.disabled=this.disabled;
	this.superstar_select.disabled=this.disabled;
	*/
}

name_t.prototype.on_selected_m=function(robot)
{
	if(!robot)
		var robot=this.get_robot();

	if(this.on_selected&&robot.school!=null&&robot.name!=null){
		this.on_selected(robot);
	}
}
