/*
 Execute student code from the "Code" tab,
 of the main robot pilot interface

 Mike Moss & Orion Lawlor, 2015-08 (Public Domain)
*/

function state_runner_t()
{
	this.execution_interval=30; // milliseconds between runs

	this.state=null;
	this.continue_state=null;
	this.continue_timeout=null;
	this.state_list=[];
	this.kill=true;
	this.state_start_time_ms=this.get_time_ms();

	this.VM_power={};
	this.VM_sensors={};
	this.VM_store={};
}

state_runner_t.prototype.run=function(robot_name,state_table)
{
	if(!state_table)
		return;

	var myself=this;

	state_table.upload_with_check(robot_name,
		function()
		{
			state_table.run();

			// Clear out old state
			myself.state=null;
			myself.continue_state=null;
			myself.continue_timeout=null;
			myself.state_list=[];
			myself.kill=false;
			state_table.clear_prints();
			myself.VM_store={};

			myself.run_m(state_table);
		});
}

state_runner_t.prototype.stop=function(state_table)
{
	//console.log("stopping");
	this.kill=true;

	// Make sure continue doesn't fire after a stop
	this.clear_continue_m();

	state_table.set_active(); // no section is active

	if (this.VM_power) { // stop the robot when the code stops running
		this.VM_power.L=this.VM_power.R=0.0; // hacky!
		if (this.onpilot) this.onpilot(this.VM_power);
	}
}

// Utility function: return time in milliseconds
state_runner_t.prototype.get_time_ms=function() {
	return (new Date()).getTime();
}

// Look up this state in our state list, or return null if it's not listed
state_runner_t.prototype.find_state=function(state_name)
{
	for(var key in this.state_list)
	{
		var s=this.state_list[key];
		if(s && s.name==state_name) {
			return s;
		}
	}
	// else not found
	return null;
}

state_runner_t.prototype.run_m=function(state_table)
{
	this.state_list=state_table.get_states();

	if(this.state_list.length<=0)
	{
		//console.log("no state_list");
		this.stop_m(state_table);
		return;
	}

	if(this.state==null)
		this.state=this.state_list[0].name;

	this.start_state(this.state);

	var myself=this;
	setTimeout(function(){myself.execute_m(state_table);},this.execution_interval);
}

// Request a stop (put actual functionality into stop, above)
state_runner_t.prototype.stop_m=function(state_table)
{
	state_table.onstop_m();
}


// Called when beginning to execute a state (either first time, or when switching states)
state_runner_t.prototype.start_state=function(state_name)
{
	//console.log("Entering VM state "+state_name);
	this.state_start_time_ms=this.get_time_ms();
}

// Inner code execution driver: prepare student-visible UI, and eval
//  Returns the virtual machine object used to wrap user code
state_runner_t.prototype.make_user_VM=function(code,states)
{
	var VM={}; // virtual machine with everything the user can access

// Block access to all parent-created members (e.g., inherited locals)
	for(var key in this)
		VM[key]=undefined;

// Import each of their state names (e.g., "start" state)
	for(var key in states)
		if(states[key])
			VM[states[key].name]=states[key].name;

// Import all needed I/O functionality
	VM.console=console;
	VM.printed_text="";
	VM.print=function(value) {
		VM.printed_text+=value+"\n";
		// console.log(value+"\n");
	};
	VM.stop=function() { VM.state=null; }

	VM.time=this.get_time_ms() - this.state_start_time_ms;

	VM.sensors=this.VM_sensors;
	VM.power=this.VM_power;
	VM.store=this.VM_store;
	VM.power_original=JSON.stringify(VM.power); // hack for change detection
	VM.robot={sensors:VM.sensors, power:VM.power};

	VM.drive=function(speed) { VM.power.L=VM.power.R=speed; };
	VM.turnleft =function(speed) { VM.power.L=-speed; VM.power.R=+speed; };
	VM.turnright=function(speed) { VM.power.L=+speed; VM.power.R=-speed; };

// Basically eval user's code here
	(new Function("with(this)\n{\n"+code+"\n}")).call(VM);
	return VM;
}

// Outer code execution driver: setup and error reporting
state_runner_t.prototype.execute_m=function(state_table)
{
	if(!this.kill)
	{
		state_table.clear_error();
		try
		{
			if(!this.state)
				throw("State is null.");

			var run_state=this.find_state(this.state);
			if(!run_state)
				throw("State \""+this.state+"\" not found!");

			// console.log("running state "+this.state);
			state_table.set_active(this.state);

			this.update_continue_m(state_table,run_state);

			var VM=this.make_user_VM(run_state.code,this.state_list);

			state_table.show_prints(VM.printed_text,this.state);

			if(VM.state===null)
			{
				//user stopped
				this.stop_m(state_table);
				return;
			}

			if(VM.state!==undefined)
			{
				if(!this.find_state(VM.state))
					throw("Next state \""+VM.state+"\" not found!");

				this.clear_continue_m();
				this.state=VM.state;
				this.start_state(VM.state);
			}

			if (JSON.stringify(VM.power)!=VM.power_original)
			{ // Send off autopilot's driving commands
				if (this.onpilot) this.onpilot(VM.power);
			}

			var myself=this;
			setTimeout(function(){myself.execute_m(state_table);},this.execution_interval);
		}
		catch(error)
		{
			//stop with error
			state_table.show_error(error,this.state);
			console.log("Error! - "+error);
			this.stop_m(state_table);
		}
	}
}

// Advance forward to the next state
state_runner_t.prototype.continue_m=function(state_table)
{
	// console.log("State advance timer callback");
	var found=false;
	var next_state=null;
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
				next_state=this.state_list[key].name;
				break;
			}
		}
	}
	console.log("State advanced from "+this.state+" to "+next_state+" due to timer");

	if(!next_state)
		this.stop_m(state_table);

	this.state=next_state;
}

// State run time limiting
state_runner_t.prototype.update_continue_m=function(state_table,state)
{
	var state_time_int=parseInt(state.time,10);
	if(!this.continue_timeout&&state_time_int>0)
	{
		var myself=this;
		this.continue_timeout=setTimeout(function(){myself.continue_m(state_table);},
			state_time_int);
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
