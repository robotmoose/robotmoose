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

	this.VM_pilot=null; // e.g., power commands, shared with manual drive
	this.VM_sensors={};
	this.VM_store={};
	this.VM_UI=null;
}

state_runner_t.prototype.set_UI=function (UI_builder) {
	this.VM_UI=UI_builder;
};

state_runner_t.prototype.run=function(robot,state_table)
{
	if(!state_table)
		return;

	var myself=this;

	state_table.upload_with_check(robot,
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

	if (this.VM_pilot) { // stop the robot when the code stops running
		this.VM_pilot.power.L=this.VM_pilot.power.R=0.0; // stop drive
		this.VM_pilot.cmd=undefined; // stop scripts
		for (var idx in this.VM_pilot.power.pwm) {
			this.VM_pilot.power.pwm[idx]=0; // stop PWM
		}
		if (this.onpilot) this.onpilot(this.VM_pilot);
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
	this.run_start_time_ms=this.get_time_ms();
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
	this.VM_UI.start_state(state_name);
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

	var time_ms=this.get_time_ms();
	VM.time=time_ms - this.state_start_time_ms; // time in state (ms)
	VM.time_run=time_ms - this.run_start_time_ms; // time since "Run" (ms)

	VM.pilot=this.VM_pilot;
	VM.pilot.cmd=undefined; // don't re-send scripts
	VM.script=function(cmd,arg) { VM.pilot.cmd={"run":cmd, "arg":arg}; }

	VM.pilot_original=JSON.stringify(VM.pilot); // hack for change detection

	VM.sensors=this.VM_sensors;
	VM.power=this.VM_pilot.power;
	VM.store=this.VM_store;
	VM.robot={sensors:VM.sensors, power:VM.power};

	VM.drive=function(speedL,speedR) { VM.power.L=speedL; VM.power.R=speedR; }

	// UI construction:
	VM.UI=this.VM_UI;
	VM.button=function(name,next_state,opts) {
		var ret=VM.UI.element(name,"button",opts);
		if (next_state && ret.oneshot) {
			console.log("UI advancing to state "+next_state+" from button "+name);
			VM.state=next_state;
			ret.oneshot=false;
		}
		return ret.value; // mouse up/down boolean
	};
	// Make a checkbox with a label
	VM.checkbox=function(name, opts) {
		var ret=VM.UI.element(name,"checkbox",opts);
		return ret.dom.checked; // checked/unchecked boolean
	};
	// Make a slider with a label
	VM.slider=function(name,min,start,max,opts){
		opts = opts ||{};
		opts.min = min;
		opts.defaultValue = start;  // starting value
		opts.max = max;
		var numSteps = 100; // number of slider steps
		opts.step = Math.abs(max - min)/numSteps; //slider step size
		var ret=VM.UI.element(name,"slider",opts);
		return parseFloat(ret.dom.value); // returns value of slider
	};
	VM.label=function(name,opts) {
		var ret=VM.UI.element(name,"label",opts);
	};

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

			if (JSON.stringify(VM.pilot)!=VM.pilot_original)
			{ // Send off autopilot's driving commands
				if (this.onpilot) this.onpilot(VM.pilot);
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
