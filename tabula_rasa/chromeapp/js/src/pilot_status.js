//callback on_connected() - Called when no pilot is connected and then at least one pilot is connected.
//callback on_disconnected() - Called when at least one pilot is connected and then no pilot is connected.

function pilot_status_t(gui,on_connected,on_disconnected)
{
	if(!gui)
		return null;
	this.gui=gui;
	this.current_pilot_heartbeat=0;
	this.on_connected=on_connected;
	this.on_disconnected=on_disconnected;
	this.prev_pilot_status=
	{
		heartbeats:-1,
		videobeats:-1
	}

	this.current_pilot_status=
	{
		heartbeats:-1,
		videobeats:-1
	}

	this.last_update_ms=new Date().getTime();
	this.pilot_connected=false;
	this.path="frontendStatus";
	this.pilot_disconnect_ms=10000;
	var _this=this;
	this.current_pilot_status_interval=setInterval(function(){_this.update();},1000);
}

pilot_status_t.prototype.check_connected=function()
{
	
	var last=this.pilot_connected;
	
	// Check if the pilot heartbeat has changed 
	// No update == no pilot connected 
	if(this.current_pilot_status.heartbeats != this.prev_pilot_status.heartbeats
		&& this.prev_pilot_status.heartbeats != -1 )
	{
		this.pilot_connected=true;
		this.last_update_ms=new Date().getTime();
		this.check_video();
	}

	if((new Date()).getTime()-this.last_update_ms>this.pilot_disconnect_ms)
		this.pilot_connected=false;

	if(!last&&this.pilot_connected&&this.on_connected)
		this.on_connected();
	if(last&&!this.pilot_connected&&this.on_disconnected)
		this.on_disconnected();
	this.gui.name.pilot_checkmark.check(this.pilot_connected);
}

pilot_status_t.prototype.check_video=function()
{
	console.log("Checking video"); //Dummy function for now
}

pilot_status_t.prototype.update_pilot=function(frontendStatus)
{
	var tmp = JSON.stringify(this.current_pilot_status);
	this.prev_pilot_status=JSON.parse(tmp);
	this.current_pilot_status.heartbeats=frontendStatus[0].heartbeats;
	this.current_pilot_status.videobeats=frontendStatus[0].videobeats;

	this.check_connected();
}
pilot_status_t.prototype.update=function()
{
	var _this=this;
	superstar_get(this.gui.name.get_robot(),this.path,function(frontendStatus){_this.update_pilot(frontendStatus);});
}


