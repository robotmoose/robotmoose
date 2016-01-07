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
	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	this.on_message=on_message;
	this.on_selected=on_selected;

	var _this=this;

	this.superstar="robotmoose.com";

	this.school=document.createElement("select");
	this.el.appendChild(this.school);
	this.school.style.width="128px";
	this.school.onchange=function(){_this.download_robots_m();};

	this.robot=document.createElement("select");
	this.el.appendChild(this.robot);
	this.robot.style.width="128px";
	this.robot.onchange=function(){_this.on_selected_m();};

	this.build_schools_m();
	this.build_robots_m();

	this.download_schools_m();
}

name_t.prototype.get_robot=function()
{
	var robot={superstar:this.superstar,school:null,name:null};

	if(this.school.selectedIndex>0&&this.robot.selectedIndex>0)
	{
		robot.school=this.school.options[this.school.selectedIndex].text;
		robot.name=this.robot.options[this.robot.selectedIndex].text;
	}

	return robot;
}




name_t.prototype.build_select_m=function(select,json,heading)
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
	}

	this.update_disables_m();
}

name_t.prototype.build_schools_m=function(json)
{
	this.build_select_m(this.school,json,"School");
	this.download_robots_m();
}

name_t.prototype.build_robots_m=function(json)
{
	this.build_select_m(this.robot,json,"Robot");
}

name_t.prototype.on_error_m=function(error)
{
	if(this.on_message)
		on_message(error);
}

name_t.prototype.download_schools_m=function()
{
	var _this=this;

	superstar_sub({superstar:this.superstar,school:"",name:""},"/",
		function(json){_this.build_schools_m(json);},
		function(error){_this.on_error_m("School download error ("+error+").");});
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

		superstar_sub({superstar:this.superstar,school:selected_school,name:""},"/",
			function(json){_this.build_robots_m(json);},
			function(error){_this.on_error_m("Robots download error ("+error+").");});
	}
}

name_t.prototype.update_disables_m=function()
{
	var school_disabled=false;
	var robot_disabled=false;

	if(this.school.selectedIndex<=0)
		robot_disabled=true;

	this.school.disabled=school_disabled;
	this.robot.disabled=robot_disabled;
}

name_t.prototype.on_selected_m=function()
{
	var robot=this.get_robot();

	if(this.on_selected&&robot.school!=null&&robot.name!=null)
		this.on_selected(robot);
}