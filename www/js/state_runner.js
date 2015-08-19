function state_runner_t()
{
	this.state=null;
	this.continue_state=null;
	this.continue_timeout=null;
	this.state_list=[];
	this.kill=true;
}

state_runner_t.prototype.run=function(state_table)
{
	this.state=null;
	this.continue_state=null;
	this.continue_timeout=null;
	this.state_list=[];
	this.kill=false;

	if(!state_table)
		return;

	this.run_m(state_table);
}

state_runner_t.prototype.stop=function(state_table)
{
	console.log("stopping");
	this.kill=true;
	this.clear_continue_m();
	state_table.set_active();
}

state_runner_t.prototype.run_m=function(state_table)
{
	this.state_list=state_table.get_states();

	if(this.state_list.length<=0)
	{
		console.log("no state_list");
		state_table.onstop_m();
		return;
	}

	if(this.state==null)
		this.state=this.state_list[0].name;

	var myself=this;
	setTimeout(function(){myself.execute_m(state_table);},10);
}

state_runner_t.prototype.execute_m=function(state_table)
{
	if(!this.kill)
	{
		try
		{
			if(!this.state)
				throw("State is null.");

			var run_state=null;

			for(var key in this.state_list)
			{
				if(this.state_list[key]&&this.state_list[key].name==this.state)
				{
					run_state=this.state_list[key];
					break;
				}
			}

			if(!run_state)
				throw("State \""+this.state+"\" not found!");

			console.log("running state "+this.state);
			state_table.set_active(this.state);

			this.update_continue_m(state_table,run_state);

			var run=(function(code,states)
			{
				var mask={};

				for(var key in this)
					mask[key]=undefined;

				for(var key in states)
					if(states[key])
						mask[states[key].name]=states[key].name;

				(new Function("with(this)\n{\n"+code+"\n}")).call(mask);
				return mask;
			})(this.state_list[key].code,this.state_list);

			if(run.state===null)
			{
				//user stopped
				state_table.onstop_m();
				return;
			}

			if(run.state!==undefined)
			{
				this.clear_continue_m();
				this.state=run.state;
			}

			var myself=this;
			setTimeout(function(){myself.execute_m(state_table);},10);
		}
		catch(error)
		{
			//stop with error
			console.log("Error! - "+error);
			state_table.onstop_m();
		}
	}
}

state_runner_t.prototype.continue_m=function(state_table)
{
	var found=false;
	this.continue_timeout=null;

	for(var key in this.state_list)
	{
		if(this.state_list[key])
		{
			if(this.state_list[key].name==this.state)
			{
				found=true;
			}
			else if(found)
			{
				this.state=this.state_list[key].name;
				break;
			}
		}
	}

	if(!found)
		state_table.onstop_m();
}

state_runner_t.prototype.update_continue_m=function(state_table,state)
{
	if(!this.continue_timeout&&parseInt(state.time,10)>0)
	{
		var myself=this;
		this.continue_timeout=setTimeout(function(){myself.continue_m(state_table);},
			parseInt(state.time,10));
	}
}

state_runner_t.prototype.clear_continue_m=function()
{
	if(this.continue_timeout)
	{
		clearTimeout(this.continue_timeout);
		this.continue_timeout=null;
	}
}