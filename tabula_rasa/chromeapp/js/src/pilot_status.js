function pilot_status_t(gui)
{
	if (!gui)
		return null;
	this.gui = gui;
	this.current_pilot_heartbeat = 0;
	this.prev_pilot_hearbeat = 0;
	this.last_update_ms = new Date().getTime();
	this.pilot_connected = false;
	this.path = "pilotHeartbeat";
	var _this = this;
	this.current_pilot_heartbeat = setInterval(function(){_this.update();},1000);

}

pilot_status_t.prototype.check_connected=function(heartbeat)
{
	console.log("Pilot heartbeat: "+heartbeat);
	this.prev_pilot_heartbeat = this.current_pilot_heartbeat;
	this.current_pilot_heartbeat = heartbeat;
	console.log("Current heartbeat: "+this.current_pilot_heartbeat);
	console.log("Prev heartbeat: "+this.prev_pilot_heartbeat);

	if(this.current_pilot_heartbeat == this.prev_pilot_heartbeat+1)
		{
			this.pilot_connected = true;
			this.last_update_ms = new Date().getTime(); 
		}
	/*else
		{
			var cur_time = new Date().getTime();
			if( cur_time - this.last_update_ms > 5000 ) //Wait 5 seconds before changing pilot status
				this.pilot_connected = false;
		}*/
	console.log("Pilot Connected " + this.pilot_connected);
}

pilot_status_t.prototype.update=function()
{
	var _this = this;
	superstar_get(this.gui.name.get_robot(),this.path, function(heartbeat){console.log(heartbeat);_this.check_connected(heartbeat.pilotHeartbeat);});
}


