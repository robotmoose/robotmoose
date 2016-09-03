function video_widget_t(obj,pilot)
{
	if(!obj)
		return null;

	this.obj=obj;

	if(pilot)
		this.pilot=pilot;

	this.update_videobeats_interval=null;

	var myself=this;

	this.obj.onactivate=function()
	{
		myself.pointer_events(true);
	}
	this.obj.ondeactivate=function()
	{
		myself.pointer_events(false);
	};
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
	this.select.onchange=function(){
		myself.download(null,myself.select.options[this.selectedIndex]);};

	this.default_link="http://robotmoose.com/webrtc/";
	this.create_option("Off",null);
	//this.create_option("WebRTC",this.default_link,"?robot=");
	this.create_option("Gruveo","https://www.gruveo.com/embed/","?code=");
}

video_widget_t.prototype.download=function(robot,option)
{
	if(robot!=null)
		this.robot=robot;

	if(this.frame)
	{
		this.bottom.removeChild(this.frame);
		this.frame=null;
	}

	if(option && option.video_link)
	{
		var myself=this;
		var robot_url=superstar.superstar+this.robot.year+this.robot.school+this.robot.name;
		robot_url=robot_url.replace(/[^A-Za-z0-9\s!?]/g,'');
		if(!this.robot.auth)
			this.robot.auth="";
		robot_url=CryptoJS.HmacSHA256(robot_url,this.robot.auth).toString(CryptoJS.enc.Hex);
		var url=option.video_link+option.video_uri+robot_url;
		this.frame=document.createElement("iframe");
		this.frame.src=url;
		this.frame.style.width="100%";
		this.frame.style.height="100%";
		this.frame.style.border="none";
		this.frame.style.overflow="hidden";
		this.bottom.appendChild(this.frame);

		if(this.obj.active)
			this.frame.style.pointerEvents="all";
		else
			this.frame.style.pointerEvents="none";
		if(this.pilot)
			this.update_videobeats_interval=setInterval(function(){myself.pilot.update_videobeats();},1000);
	}

	if(this.pilot&& option&&!option.video_link)
		clearInterval(this.update_videobeats_interval);

}

video_widget_t.prototype.create_option=function(name,link,uri)
{
	var option=document.createElement("option");
	option.text=name;
	option.video_link=link;
	option.video_uri=uri;
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
