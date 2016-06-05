/*
	Arsh Chauhan
	06/03/2016
	Public Domain
*/

/*
	pilot_status.js
	Class to handle pilot status. 
	Increments a counter as long as a pilot is connected
	Used for backend pilot detetcion   
*/

//TO DO: Detect multiple pilots.  

function pilot_status_t(ui)
{
	this.current_heartbeat = 0;
	this.ui = ui;
	this.path = "pilotHeartbeat";
	var _this = this;

	setInterval(function(){_this.upload();},1000);

}

pilot_status_t.prototype.upload=function()
{
	if(!this.ui || !this.ui.robot || !this.ui.robot.name)
		return null;

	this.current_heartbeat++
	if(this.current_heartbeat > 255)
		this.current_heartbeat = 0;
	console.log("Pilot Heartbeat: " + this.current_heartbeat);
	superstar_set(this.ui.robot,this.path,{"pilotHeartbeat":this.current_heartbeat});
}
