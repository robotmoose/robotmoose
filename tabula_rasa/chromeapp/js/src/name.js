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

	this.school=document.createElement("select");
	this.el.appendChild(this.school);
	this.school.style.width="128px";
	this.school.onchange=function(){_this.download_robots_m();};

	this.robot=document.createElement("select");
	this.el.appendChild(this.robot);
	this.robot.style.width="128px";
	this.robot.onchange=function(){_this.on_selected_m();};

	this.superstar=document.createElement("select");
	this.el.appendChild(this.superstar);
	this.superstar.style.width="128px";
	this.superstar.onchange=function()
	{
		if(this.selectedIndex!=0)
		{
			_this.build_schools_m();
			_this.build_robots_m();
		}
		else
		{
			_this.build_select_m(_this.schools,[],"School","School");
			_this.build_select_m(_this.robots,[],"Robot","Robot");
		}
	};


	_this.school_interval=setInterval(function()
	{
		if(!_this.superstar.disabled)
		{
				_this.on_loaded_robot=
				{
					superstar:_this.superstar.options[_this.superstar.selectedIndex].text,
					school:_this.school.options[_this.school.selectedIndex].text,
					name:_this.robot.options[_this.robot.selectedIndex].text
				};
				_this.download_schools_m();
		}
	},1000);

	_this.build_superstar_m();
	_this.build_schools_m();
	_this.build_robots_m();

	_this.disables_interval=setInterval(function(){_this.update_disables_m();},100);
	_this.download_schools_m();
}


name_t.prototype.destroy=function()
{
	clearInterval(this.disables_interval);
	clearInterval(this.school_interval);
	this.div.removeChild(this.el);
}

name_t.prototype.get_robot=function()
{
	var robot={superstar:"robotmoose.com",school:null,name:null};

	if(this.school.selectedIndex>0&&this.robot.selectedIndex>0)
	{
		robot.school=this.school.options[this.school.selectedIndex].text;
		robot.name=this.robot.options[this.robot.selectedIndex].text;
		robot.superstar=this.superstar.options[this.superstar.selectedIndex].text;
	}

	return robot;
}

name_t.prototype.reload=function()
{
	this.build_superstar_m();
	this.build_schools_m();
	this.build_robots_m();
}




name_t.prototype.build_select_m=function(select,json,heading,on_loaded_value)
{
	select.length=0;

	var heading_option=document.createElement("option");
	select.appendChild(heading_option);
	heading_option.text=heading;

	for(var key in json)
	{
		var option=document.createElement("option");
		select.appendChild(option);
		option.text=json[key];

		if(json[key]==on_loaded_value)
		{
			select.value=on_loaded_value;

			if(select.onchange)
				select.onchange();
		}
	}

	this.update_disables_m();
}

name_t.prototype.build_schools_m=function(json)
{
	this.build_select_m(this.school,json,"School",this.on_loaded_robot.school);
	this.download_robots_m();
}

name_t.prototype.build_robots_m=function(json)
{
	this.build_select_m(this.robot,json,"Robot",this.on_loaded_robot.name);
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

	this.build_select_m(this.superstar,superstar_options,"Superstar",this.on_loaded_robot.superstar);
}

name_t.prototype.on_error_m=function(error)
{
	if(this.on_message)
		this.on_message(error);
}

name_t.prototype.download_schools_m=function()
{
	if(this.superstar.selectedIndex>0)
	{
		var _this=this;
		superstar_sub({superstar:this.on_loaded_robot.superstar,school:"",name:""},"/",
			function(json){_this.build_schools_m(json); ;},
			function(error){_this.on_error_m("School download error ("+error+").");});
	}
}

name_t.prototype.download_robots_m=function()
{
	if(this.school.selectedIndex<=0)
	{
		this.build_robots_m();
		return;
	}


	var selected_school=this.school.options[this.school.selectedIndex].text;

	if(selected_school)
	{
		var _this=this;

		superstar_sub({superstar:this.superstar.options[this.superstar.selectedIndex].text,school:selected_school,name:""},"/",
			function(json){_this.build_robots_m(json);},
			function(error){_this.on_error_m("Robots download error ("+error+").");});
	}
}

name_t.prototype.update_disables_m=function()
{
	var disabled=false;
	if(this.school.selectedIndex<=0||this.disabled)
		disabled=true;
	this.robot.disabled=disabled;

	this.school.disabled=this.disabled;
	this.superstar.disabled=this.disabled;
}

name_t.prototype.on_selected_m=function()
{
	var robot=this.get_robot();

	if(this.on_selected&&robot.school!=null&&robot.name!=null)
		this.on_selected(robot);
}
