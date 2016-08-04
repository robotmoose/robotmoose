//callback onconnect() - Called when no pilot is connected and then at least one pilot is connected.
//callback ondisconnect() - Called when at least one pilot is connected and then no pilot is connected.
//callback onchange(count) - Gives number of pilots when DOWNLOADED or TIMEDOUT.
//callback onvideohangup(pilot_uuid) - Called when main pilot hangs up (uuid maybe null if no pilot is connected).
//callback onvideocall(pilot_uuid) - Called when main pilot calls in.
var blarg=30;
function pilot_status_t(name,pilot_checkmark,onconnect,ondisconnect)
{
	if(!name||!pilot_checkmark)
		return null;
	this.name=name;
	this.pilot_checkmark=pilot_checkmark;
	this.onconnect=onconnect;
	this.ondisconnect=ondisconnect;

	this.pilots={};
	this.pilot_timers={};
	this.reset();
	this.current_pilot=null;

	var _this=this;
	this.timeout_time=3000;
	this.timeout=null;
	this.video_timer=null;
	this.videohungup=false;
	this.can_call_again=true;

	var handle_get=function(data)
	{
		var robot=_this.name.get_robot();
		if(valid_robot(robot))
			superstar.get_next(robot_to_starpath(robot)+
				"frontend_status",function(data)
				{
					if(_this.timeout)
					{
						clearTimeout(_this.timeout);
						_this.timeout=null;
					}
					_this.timeout=setTimeout(function(){_this.disconnect();},_this.timeout_time);
					_this.update(data);
					handle_get();
				},
				function(error)
				{
					console.log(error);
					setTimeout(handle_get,100);
				});
		else
		{
			setTimeout(handle_get,100);
		}
	};
	handle_get();

	this.video_interval=setInterval(function()
	{
		//Get webview number of video elements.
		var webview=document.querySelector("webview");
		webview.executeScript
		(
			{code:"chrome.storage.local.set({robotmoose_pilot_status:document.getElementsByTagName('video').length});"},
			function(results)
			{
				if(!chrome.runtime.lastError)
				{
					chrome.storage.local.get("robotmoose_pilot_status",function(result)
					{
						var time=Date.now();

						//No heartbeats in a while, hangup.
						if(_this.current_pilot&&(time-_this.video_timer)>_this.timeout_time&&!_this.videohungup)
						{
							_this.video_timer=time+_this.timeout_time*1.5;
							_this.current_pilot=null;
							_this.videohungup=true;
							if(_this.onvideohangup)
								_this.onvideohangup(_this.current_pilot);
						}

						//Heartbeats and hungup, call.
						if(_this.current_pilot&&_this.videohungup)
						{
							_this.videohungup=false;
							if(_this.onvideocall)
								_this.onvideocall(_this.current_pilot,null);
						}

						//User hangups on their side.
						if(result.robotmoose_pilot_status<=0&&!_this.videohungup&&_this.can_call_again)
						{
							//Call
							if(_this.onvideocall)
								_this.onvideocall(_this.current_pilot,null);

							//Put repeating calls in a timeout to prevent infinite calling.
							_this.can_call_again=false;
							setTimeout(function()
							{
								if(!_this.can_call_again)
									_this.can_call_again=true;
							},_this.timeout_time*1.5);
						}
					})
				}
			}
		);
	},1000);
}

pilot_status_t.prototype.destroy=function()
{
	if(this.video_interval)
	{
		clearInterval(this.video_interval);
		this.video_interval=null;
	}

	if(this.el)
	{
		this.div.removeChild(this.el);
		this.el=this.div=null;
	}
}

pilot_status_t.prototype.disconnect=function()
{
	this.reset();
	this.pilot_checkmark.check(this.pilot_connected);
	if(this.ondisconnect)
		this.ondisconnect();
	if(this.onchange)
		this.onchange(0);
	var robot=this.name.get_robot();
	if(valid_robot(robot))
		superstar.set(robot_to_starpath(robot)+"frontend_status",{});
}

pilot_status_t.prototype.reset=function()
{
	this.num_pilots=0;
	this.num_video=0;
	this.pilot_connected=false;
	this.pilot_timers={};
}

pilot_status_t.prototype.update=function(new_pilots)
{
	var num_pilots=0;
	var num_video=0;
	var active_pilots={};
	var old_pilots={};
	var elderest_pilot=null;

	//Time for checking if we have pilots to remove.
	var time=Date.now();

	//There are new pilots.
	if(new_pilots&&JSON.stringify(this.pilots)!=JSON.stringify(new_pilots))
	{
		//Go through pilots that are online, check them against our pilots.
		for(var ii in new_pilots)
		{
			if(ii in this.pilots)
			{
				//New heartbeats, add pilots and set timeout.
				if(new_pilots[ii].heartbeats!=this.pilots[ii].heartbeats)
				{
					++num_pilots;
					active_pilots[ii]=new_pilots[ii];
					if(!this.pilot_timers)
						this.pilot_timers={};
					this.pilot_timers[ii]=time;
				}

				//No new heartbeats, but get_next only sees a single pilots change,
				//  check timeout (or else you will always have 1 pilot!).
				else if((time-this.pilot_timers[ii])<this.timeout_time)
				{
					++num_pilots;
					active_pilots[ii]=new_pilots[ii];
				}

				//New video heartbeats, update elderest pilot.
				if(new_pilots[ii].videobeats!=this.pilots[ii].videobeats)
				{
					++num_video;
					if(!elderest_pilot||ii<elderest_pilot)
						elderest_pilot=ii;
					if(ii==this.current_pilot)
						this.video_timer=time;
				}
			}
			else
			{
				active_pilots[ii]=new_pilots[ii];
			}
		}

		//Keep old pilots.
		old_pilots=JSON.parse(JSON.stringify(this.pilots));

		//Set current pilots to those that are online.
		this.pilots=JSON.parse(JSON.stringify(new_pilots));
	}

	//Set new pilot if old one is gone.
	if(!this.current_pilot&&elderest_pilot!=this.current_pilot)
	{
		this.current_pilot=elderest_pilot;
		this.video_timer=time;
	}

	//Go through robots we already have, check to see if they are disconnected
	//  and remove them from online (keeps the online array from getting too big).
	//  This also resets the pilot.
	var robot=this.name.get_robot();
	for(var ii in old_pilots)
		if(!(ii in active_pilots)&&(this.pilot_timers[ii]||(time-this.pilot_timers[ii])>this.timeout_time))
		{
			superstar.set(robot_to_starpath(robot)+"frontend_status/"+ii,null);
			if(ii==this.current_pilot)
				this.current_pilot=null;
		}

	////Clear up out local timers (probably not needed...but it makes me sleep better...).
	//var new_pilot_timers={};
	//for(var ii in this.pilot_timers)
	//	if((time-this.pilot_timers[ii])<this.timeout_time*2)
	//		new_pilot_timers[ii]=this.pilot_timers[ii];
	//this.pilot_timers=new_pilot_timers;

	//Update checkmark and such.
	var old_pilot_connected=this.pilot_connected;
	this.pilot_connected=(num_pilots!=0);
	this.pilot_checkmark.check(this.pilot_connected);

	//Had no pilots and now we have at least one, connect!
	if(old_pilot_connected!=this.pilot_connected&&
		this.pilot_connected&&this.onconnect)
		this.onconnect();

	//Change the number of pilots.
	if(this.onchange)
		this.onchange(num_pilots);
}


