function name_t(div )
{
	if(!div)
		return null;

	this.div=div;
	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	var _this=this;

	this.school=document.createElement("select");
	this.el.appendChild(this.school);
	this.school.onchange=function(){_this.download_robots_m();};

	this.robot=document.createElement("select");
	this.el.appendChild(this.robot);
	this.school.onchange=function(){};
}

name_t.prototype.download_schools_m=function()
{
	//download schools...
}

name_t.prototype.download_robots_m=function()
{
	var selected_school=this.school.options[this.school.selectedIndex];

	if(selected_school)
	{
		//download robots....
	}
}