function pilot_status_t(gui)
{
	if(!gui)
		return null;
	this.gui=gui;
	this.current_pilot_heartbeat=0;
	this.prev_pilot_hearbeat=0;
	this.last_update_ms=new Date().getTime();
	this.pilot_connected=false;
	this.path="pilotHeartbeat";
	this.pilot_disconnect_ms=5000;
	var _this=this;
	this.current_pilot_heartbeat=setInterval(function(){_this.update();},1000);

}

pilot_status_t.prototype.check_connected=function(heartbeat)
{
	this.prev_pilot_heartbeat=this.current_pilot_heartbeat;
	this.current_pilot_heartbeat=heartbeat;

	if(this.current_pilot_heartbeat==this.prev_pilot_heartbeat+1)
	{
		this.pilot_connected=true;
		this.last_update_ms=new Date().getTime();
	}

	if((new Date()).getTime()-this.last_update_ms>this.pilot_disconnect_ms)
		this.pilot_connected=false;
}

pilot_status_t.prototype.update=function()
{
	var _this=this;
	superstar_get(this.gui.name.get_robot(),this.path,function(heartbeat){_this.check_connected(heartbeat.pilotHeartbeat);});
}


