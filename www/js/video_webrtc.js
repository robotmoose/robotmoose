function video_webrtc_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.div.style.width="100%";
	this.div.style.height="100%";
	this.div.style.overflow="hidden";

	var myself=this;
	this.robot=null;

	this.select=document.createElement("select");
	this.div.appendChild(this.select);
	this.select.className="form-control";
	this.select.onchange=function(){myself.download(null, this.options[this.selectedIndex].video_link);};

	this.create_option("webRTC","http://www.robotmoose.com/webrtc/");
	this.create_option("gruveo","https://www.gruveo.com/embed/");
}

video_webrtc_t.prototype.download=function(robot, link)
{
	if(robot != null)
	{
		this.robot = robot;
	}

	if(this.frame)
	this.element.removeChild(this.frame);

	this.frame=document.createElement("iframe");
	this.frame.src=link + "?robot=" + this.robot.name;
	this.frame.style.width="100%";
	this.frame.style.height="100%";
	this.frame.style.border="none";
	this.frame.style.padding="0px";
	this.frame.style.margin="0px";
	this.frame.style.overflow="hidden";
	this.frame.style.pointerEvents="none";
	this.div.appendChild(this.frame);
}

video_webrtc_t.prototype.create_option=function(name, link)
{
	var option = document.createElement("option");

	option.video_link=link;
	option.text = name;

	this.select.appendChild(option);

}