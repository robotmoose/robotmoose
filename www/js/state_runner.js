/*
 Execute student code from the "Code" tab,
 of the main robot pilot interface

 Mike Moss & Orion Lawlor, 2015-08 (Public Domain)
*/

function state_runner_t()
{
	var _this=this;
	this.execution_interval=30; // milliseconds between runs

	this.state=null;
	this.continue_state=null;
	this.continue_timeout=null;
	this.state_list=[];
	this.kill=true;
	this.state_start_time_ms=this.get_time_ms();

	var myself=this;
	this.VM={};
	this.VM_pilot=null; // e.g., power commands, shared with manual drive
	this.VM_pilot_last="???";
	this.VM_sensors={};
	this.VM_store={};
	this.VM_UI=null;

	// Send off the pilot data, if it has changed
	myself.pilot_flush=function() {
		console.log("Pilot flush: "+myself.VM_pilot.power.L);
		var pilot_cur=JSON.stringify(myself.VM_pilot);
		if (pilot_cur!=myself.VM_pilot_last)
		{ // Send off autopilot's driving commands
			myself.VM_pilot_last=pilot_cur;
			if (myself.onpilot) myself.onpilot(myself.VM_pilot);
		}
	}

	// Commit all changed data
	myself.do_writes=function(VM) {
		myself.pilot_flush();

		VM.state_written=VM.state;
	}

	/**
	 seq is used to sequence commands, including delays.
	 The trick is executing code before and after the delay,
	 but we disable side effects on the part of the code (the "phase")
	 that is not currently active.
	*/
	this.VM_seq={};
	var seq=this.VM_seq;

	// Storage for blocking functions.  Index is by code_count.
	seq.store=[];

	// Restart the sequence for a new run
	seq.reset=function() {
		seq.code_count=0; // source code phase (counts up every time through code)
		seq.exec_count=0; // active runtime phase (counts up only after delays)
		seq.exec_start_count=-1; // trails exec_count by 1
	};
	seq.reset();

	// Check if we are we currently active (running)
	seq.current=function() { return seq.code_count==seq.exec_count; };

	// Mark start of a potentially blocking phase.
	//  Returns state storage for this phase.
	seq.block_start=function(VM) {
		if (seq.current() && seq.exec_start_count<seq.exec_count)
		{ // This is the first run of the new phase
			seq.exec_start_count=seq.exec_count;
			seq.store[seq.code_count]={}; // new empty object

			// Commit user's actions before blocking
			myself.do_writes(VM);
		}
		return seq.store[seq.code_count];
	}

	// Advance to the next piece of code
	seq.advance=function() {
		seq.exec_count++;
	}

	// Mark end of blocking section
	seq.block_end=function() {
		seq.code_count++; // increment count for each phase
	}
}

state_runner_t.prototype.set_UI=function (UI_builder) {
	this.VM_UI=UI_builder;
};

state_runner_t.prototype.run=function(robot,state_table)
{
	if(!state_table)
		return;

	var myself=this;

	state_table.upload(robot);

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
}

state_runner_t.prototype.stop=function(state_table)
{
	//console.log("stopping");
	this.kill=true;

	// Make sure continue doesn't fire after a stop
	this.clear_continue_m();

	state_table.set_active(); // no section is active

	this.VM_seq.reset(); // reset sequencer

	if (this.VM_pilot) { // stop the robot when the code stops running
		this.VM_pilot.power.L=this.VM_pilot.power.R=0.0; // stop drive
		this.pilot_flush();
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
	this.VM_seq.reset();
	//console.log("Entering VM state "+state_name);
	this.state_start_time_ms=this.get_time_ms();
}

// Inner code execution driver: prepare student-visible UI, and eval
//  Returns the virtual machine object used to wrap user code
state_runner_t.prototype.make_user_VM=function(code,states)
{
	var myself=this;
	var VM={}; // virtual machine with everything the user can access

// Block access to all parent-created members (e.g., inherited locals)
	for(var key in this)
		VM[key]=undefined;

// Import each of their state names (e.g., "start" state)
	for(var key in states)
		if(states[key])
			VM[states[key].name]=states[key].name;

// Sequencer
	VM.sequencer=this.VM_seq;
	VM.sequencer.code_count=0;

// Import all needed I/O functionality
	VM.console=console;
	VM.printed_text="";
	VM.print=function(value) {
		if (VM.sequencer.current()) {
			VM.printed_text+=value+"\n";
			// console.log(value+"\n");
		}
	};
	VM.stop=function() {
		if (VM.sequencer.current()) {
			VM.state=null;
		}
	}

	var time_ms=this.get_time_ms();
	VM.time=time_ms - this.state_start_time_ms; // time in state (ms)
	VM.time_run=time_ms - this.run_start_time_ms; // time since "Run" (ms)

	VM.delay=function(ms) {
		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			if (!t.time)
			{ // starting a delay
				t.time=VM.time+ms;
			}
			if (VM.time >= t.time)
			{ // done with delay
				VM.sequencer.advance();
			}
		}
		VM.sequencer.block_end();
	}

	VM.pilot=JSON.parse(JSON.stringify(this.VM_pilot));
	VM.pilot.cmd=undefined; // don't re-send scripts
	VM.script=function(cmd,arg) {
		if (VM.sequencer.current()) {
			VM.pilot.cmd={"run":cmd, "arg":arg};
		}
	};

	VM.sensors=this.VM_sensors;
	VM.power=this.VM_pilot.power;
	VM.store=this.VM_store;
	VM.robot={sensors:VM.sensors, power:VM.power};

	if (VM.sensors.power && VM.sensors.power.neopixel && !VM.power.neopixel) {
		VM.power.neopixel=JSON.parse(JSON.stringify(VM.sensors.power.neopixel));
	}

	// Simple instantanious drive:
	VM.drive=function(speedL,speedR) {
		if (VM.sequencer.current()) {

			// cap speed at 100 percent
			if (speedL > 100.0) speedL = 100.0;
			else if (speedL < -100.0) speedL = -100.0;
			if (speedR > 100.0) speedR = 100.0;
			else if (speedR < -100.0) speedR = -100.0;

			VM.power.L=speedL; VM.power.R=speedR;
		}
	}

	// Drive forward specified distance (cm)
	VM.forward=function(target,speed) {
		if (!target) target=10; // centimeters
		if (!speed) speed=0.4*100; // moderate speed -- scaled for percentage
		else if (speed > 100.0) speed = 100.0; // cap speed at 100 percent
		else if (speed < -100.0) speed = -100.0;

		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			var p=new vec3(VM.sensors.location.x,VM.sensors.location.y,0.0);
			if (!t.start)
			{ // starting a move
				t.start=p;
			}
			if (target<0) { // drive backwards
				target=-target;
				speed=-speed;
			}
			var dist=target - 100.0*p.distanceTo(t.start);
			var slow_dist=10.0; // scale back on approach
			if (dist<slow_dist) speed*=0.1+0.9*dist/slow_dist;
			console.log("Forward: distance: "+dist+" -> speed "+speed);
			VM.power.L=VM.power.R=speed;
			if (dist <= 0.0)
			{ // done with move
				VM.power.L=VM.power.R=0.0;
				VM.sequencer.advance();
			}
			// Commit these new power values:
			myself.do_writes(VM);
		}
		VM.sequencer.block_end();
	}
	VM.backward=function(target,speed) {
		if (!target) target=10; // centimeters
		VM.forward(-target,speed);
	}

	// Turn right (clockwise) specified distance (deg)
	VM.right=function(target,speed) {
		if (!target) target=90; // degrees
		if (!speed) speed=0.3*100; // --- scaled for percentage
		else if (speed > 100.0) speed = 100.0; // cap speed at 100 percent
		else if (speed < -100.0) speed = -100.0;

		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			var a=VM.sensors.location.angle;
			if (!t.target)
			{ // starting turn: compute target angle
				t.target=a-target;
			}
			var dist=a-t.target;
			while (dist>+180.0) dist-=360.0; // reduce mod 360
			while (dist<-180.0) dist+=360.0;
			if (target<0) { // drive backwards to turn other way
				speed=-speed;
				dist=-dist;
			}

			var slow_dist=40.0; // scale back on approach
			if (dist<slow_dist) speed*=0.1+0.9*dist/slow_dist;
			console.log("Turn: distance: "+dist+" -> speed "+speed);
			VM.power.L=+speed; VM.power.R=-speed;
			if (dist <= 0.0)
			{ // done with move
				VM.power.L=VM.power.R=0.0;
				VM.sequencer.advance();
			}
			// Commit these new power values:
			myself.do_writes(VM);
		}
		VM.sequencer.block_end();
	}
	VM.left=function(target,speed) {
		if (!target) target=90; // degrees
		VM.right(-target,speed);
	}



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
	(new Function(
		"with(this)\n{\n"+  // "with" lets us access VM. stuff directly
			"(function() { \"use strict\";\n"+	// strict mode prevents undefined variables
				code+	// user's code
			"}())\n"+
		"\n}"
	)).call(VM);

	if (VM.sequencer.current()) this.do_writes(VM);

	return VM;
}

// Outer code execution driver: setup and error reporting
state_runner_t.prototype.execute_m=function(state_table)
{
	var _this=this;
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

			var VM=this.make_user_VM(run_state.code+"\n",this.state_list);

			if (VM.sequencer.exec_count>=VM.sequencer.code_count)
			{ // Restart the state if we're at the end of the sequence:
				console.log("Resetting sequencer back to start");
				_this.do_writes(_this.VM);
				VM.sequencer.reset();
			}

			state_table.show_prints(VM.printed_text,this.state);

			if(VM.state_written===null)
			{
				//user stopped
				this.stop_m(state_table);
				return;
			}

			if(VM.state_written!==undefined)
			{
				if(!this.find_state(VM.state_written))
					throw("Next state \""+VM.state_written+"\" not found!");

				this.clear_continue_m();
				this.state=VM.state_written;
				this.start_state(VM.state_written);
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
