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
	this.prev_pilot_hearbeat=0;
	this.last_update_ms=new Date().getTime();
	this.pilot_connected=false;
	this.path="pilotHeartbeat";
	this.pilot_disconnect_ms=10000;
	var _this=this;
	this.current_pilot_heartbeat=setInterval(function(){_this.update();},1000);
}

pilot_status_t.prototype.check_connected=function(heartbeat)
{
	this.prev_pilot_heartbeat=this.current_pilot_heartbeat;
	this.current_pilot_heartbeat=heartbeat;

	var last=this.pilot_connected;

	// +1 is needed since the heartbeat does not update if there is no pilot. 
	//No update == no pilot connected 
	if(this.current_pilot_heartbeat==this.prev_pilot_heartbeat+1)
	{
		this.pilot_connected=true;
		this.last_update_ms=new Date().getTime();

	}

	if((new Date()).getTime()-this.last_update_ms>this.pilot_disconnect_ms)
		this.pilot_connected=false;

	if(!last&&this.pilot_connected&&this.on_connected)
		this.on_connected();
	if(last&&!this.pilot_connected&&this.on_disconnected)
		this.on_disconnected();
	this.gui.name.pilot_checkmark.check(this.pilot_connected);
}

pilot_status_t.prototype.update=function()
{
	var _this=this;
	superstar_get(this.gui.name.get_robot(),this.path,function(heartbeat){_this.check_connected(heartbeat.pilotHeartbeat);});
}


