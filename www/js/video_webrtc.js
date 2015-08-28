function video_webrtc_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.div.style.width="100%";
	this.div.style.height="100%";
	this.div.style.overflow="hidden";
}

video_webrtc_t.prototype.download=function(robot)
{
	if(this.frame)
		this.element.removeChild(this.frame);

	this.frame=document.createElement("iframe");
	this.frame.src="http://test.robotmoose.com/webrtc/?robot="+robot.name;
	this.frame.style.width="100%";
	this.frame.style.height="100%";
	this.frame.style.border="none";
	this.frame.style.padding="0px";
	this.frame.style.margin="0px";
	this.frame.style.overflow="hidden";
	this.frame.style.pointerEvents="none";
	this.div.appendChild(this.frame);
}