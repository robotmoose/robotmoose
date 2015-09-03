function video_widget_t(obj)
{
	if(!obj)
		return null;

	this.obj=obj;
	var myself=this;

	this.obj.onactivate=function(){myself.pointer_events(true);};
	this.obj.ondeactivate=function(){myself.pointer_events(false);};
	this.obj.resizer.onresizing=function(){myself.pointer_events(false);};

	this.table=document.createElement("table");
	this.table.style.margin="0px";
	this.table.style.padding="0px";
	this.table.style.width="100%";
	this.table.style.height="100%";
	this.top_row=this.table.insertRow(0);
	this.top_row.style.margin="0px";
	this.top_row.style.padding="0px";
	this.top_row.style.width="100%";
	this.top=this.top_row.insertCell(0);
	this.top.style.margin="0px";
	this.top.style.padding="0px";
	this.top.style.width="100%";
	this.bottom_row=this.table.insertRow(1);
	this.bottom_row.style.margin="0px";
	this.bottom_row.style.padding="0px";
	this.bottom_row.style.width="100%";
	this.bottom_row.style.height="100%";
	this.bottom=this.bottom_row.insertCell(0);
	this.bottom.style.margin="0px";
	this.bottom.style.padding="0px";
	this.bottom.style.width="100%";
	this.bottom.style.height="100%";
	this.obj.content.appendChild(this.table);

	this.robot=null;

	this.select=document.createElement("select");
	this.top.appendChild(this.select);
	this.select.className="form-control";
	this.select.onchange=function(){myself.download(null,myself.select.options[this.selectedIndex].video_link);};

	this.default_link="http://robotmoose.com/webrtc/";
	this.create_option("Off",null);
	this.create_option("WebRTC",this.default_link);
	this.create_option("Gruveo","https://www.gruveo.com/embed/");
}

video_widget_t.prototype.download=function(robot,link)
{
	if(robot!=null)
		this.robot=robot;

	if(this.frame)
	{
		this.bottom.removeChild(this.frame);
		this.frame=null;
	}

	if(link)
	{
		this.frame=document.createElement("iframe");
		this.frame.src=link+"?robot="+this.robot.name;
		this.frame.style.width="100%";
		this.frame.style.height="100%";
		this.frame.style.border="none";
		this.frame.style.overflow="hidden";

		if(this.obj.active)
			this.frame.style.pointerEvents="all";
		else
			this.frame.style.pointerEvents="none";

		this.bottom.appendChild(this.frame);
	}
}

video_widget_t.prototype.create_option=function(name,link)
{
	var option=document.createElement("option");
	option.video_link=link;
	option.text=name;
	this.select.appendChild(option);
}

video_widget_t.prototype.pointer_events=function(value)
{
	if(this.frame)
	{
		this.frame.style.pointerEvents="none";

		if(value&&!this.obj.resizer.dragging.on)
			this.frame.style.pointerEvents="all";
	}
}